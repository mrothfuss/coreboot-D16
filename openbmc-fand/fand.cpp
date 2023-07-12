/*
 * fand
 *
 * Copyright 2017 Raptor Engineering, LLC
 * Copyright 2014-present Facebook. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Daemon to manage the fan speed to ensure that we stay within a reasonable
 * temperature range.  We're using a simplistic algorithm to get started:
 *
 * If the fan is already on high, we'll move it to medium if we fall below
 * a top temperature.  If we're on medium, we'll move it to high
 * if the temperature goes over the top value, and to low if the
 * temperature falls to a bottom level.  If the fan is on low,
 * we'll increase the speed if the temperature rises to the top level.
 *
 * To ensure that we're not just turning the fans up, then back down again,
 * we'll require an extra few degrees of temperature drop before we lower
 * the fan speed.
 *
 * We check the RPM of the fans against the requested RPMs to determine
 * whether the fans are failing, in which case we'll turn up all of
 * the other fans and report the problem..
 *
 * TODO:  Implement a PID algorithm to closely track the ideal temperature.
 * TODO:  Determine if the daemon is already started.
 */

/* Yeah, the file ends in .cpp, but it's a C program.  Deal. */

/* XXX:  Both CONFIG_WEDGE and CONFIG_WEDGE100 are defined for Wedge100 */

#if !defined(CONFIG_ASUS)
#error "ASUS hardware platform defined!"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <syslog.h>

#define CONFIG_SLOPPY_FANS 1

#include "watchdog.h"

/* Sensor definitions */

#define INTERNAL_TEMPS(x) ((x) * 1000.0) // stored as C * 1000
#define EXTERNAL_TEMPS(x) ((x) / 1000.0)

/*
 * The sensor for the uServer CPU is not on the CPU itself, so it reads
 * a little low.  We are special casing this, but we should obviously
 * be thinking about a way to generalize these tweaks, and perhaps
 * the entire configuration.  JSON file?
 */

#define USERVER_TEMP_FUDGE INTERNAL_TEMPS(0)

#define BAD_TEMP INTERNAL_TEMPS(-60)
#define BAD_VOLTAGE (-50)

#define BAD_READ_THRESHOLD 4    /* How many times can reads fail */
#define FAN_FAILURE_THRESHOLD 4 /* How many times can a fan fail */
#define FAN_SHUTDOWN_THRESHOLD 20 /* How long fans can be failed before */
/* we just shut down the whole thing. */

#define PWM_DIR "/sys/class/i2c-adapter/i2c-1/1-002f/"

#define FAN_READ_RPM_FORMAT "fan%d_input"
#define PWM_UNIT_MAX 255

#define CHIPSET_TEMP_DEVICE PWM_DIR "temp1_input"
#define CPU1_TEMP_DEVICE PWM_DIR "temp7_input"
#define CPU2_TEMP_DEVICE PWM_DIR "temp8_input"
#define CPU2_VCORE_DEVICE PWM_DIR "in1_input"

#define MAIN_POWER_OFF_DIRECTION "/sys/class/gpio/gpio40/direction"
#define MAIN_POWER_OFF_CTL "/sys/class/gpio/gpio40/value"

#define FAN_LED_RED "1"
#define FAN_LED_BLUE "0"

#define LARGEST_DEVICE_NAME 120

#define REPORT_TEMP 720  /* Report temp every so many cycles */

/* Sensor limits and tuning parameters */

#define INTAKE_LIMIT INTERNAL_TEMPS(60)
#define SWITCH_LIMIT INTERNAL_TEMPS(80)
#define CHIPSET_LIMIT INTERNAL_TEMPS(80)
#define USERVER_LIMIT INTERNAL_TEMPS(90)

#define CPU_LIMIT INTERNAL_TEMPS(68)

#define TEMP_TOP INTERNAL_TEMPS(70)
#define TEMP_BOTTOM INTERNAL_TEMPS(40)

/*
 * Toggling the fan constantly will wear it out (and annoy anyone who
 * can hear it), so we'll only turn down the fan after the temperature
 * has dipped a bit below the point at which we'd otherwise switch
 * things up.
 */

#define COOLDOWN_SLOP INTERNAL_TEMPS(3)

#define WEDGE_FAN_LOW 35
#define WEDGE_FAN_MEDIUM 50
#define WEDGE_FAN_HIGH 70
#define WEDGE_FAN_MAX 100

#define SIXPACK_FAN_LOW 35
#define SIXPACK_FAN_MEDIUM 55
#define SIXPACK_FAN_HIGH 75
#define SIXPACK_FAN_MAX 99

/*
 * Mapping physical to hardware addresses for fans;  it's different for
 * RPM measuring and PWM setting, naturally.  Doh.
 */

float fan_scale_speed_map[] = {0.980, 1.021, 0.963, 1.0, 1.091, 1.0, 0.961, 1.0};	// Chassis fans may have a different max rpm than 2000; scale the stock profile accordingly.
int fan_population_map[] = {1, 1, 1, 0, 1, 0, 1, 0};	// 1 == fan populated, 0 == fan disconnected
int fan_to_rpm_map[]     = {1, 2, 3, 4, 5, 6, 7, 8};
int fan_to_pwm_map[]     = {1, 1, 2, 2, 2, 2, 2, 2};	// 1 == 4 pin fans, 2 == 3 pin fans
#define FANS 8



/*
 * The measured RPM of the fans doesn't match linearly to the requested
 * rate.  In addition, there are coaxially mounted fans, so the rear fans
 * feed into the front fans.  The rear fans will run slower since they're
 * grabbing still air, and the front fants are getting an extra boost.
 *
 * We'd like to measure the fan RPM and compare it to the expected RPM
 * so that we can detect failed fans, so we have a table (derived from
 * hardware testing):
 */

struct rpm_to_pct_map {
	uint pct;
	uint rpm;
};

struct rpm_to_pct_map rpm_front_map[] = {
	{0, 2025},
	{5, 2026},
	{10, 2027},
	{15, 2028},
	{20, 2082},
	{25, 2336},
	{30, 2777},
	{35, 3211},
	{40, 3685},
	{45, 4123},
	{50, 4574},
	{55, 4922},
	{60, 5338},
	{65, 5759},
	{70, 6208},
	{75, 6635},
	{80, 7106},
	{85, 7564},
	{90, 7989},
	{95, 8492},
	{100, 8626}};
#define FRONT_MAP_SIZE (sizeof(rpm_front_map) / sizeof(struct rpm_to_pct_map))

struct rpm_to_pct_map rpm_rear_map[] = {
	{0, 2000},
	{5, 2003},
	{10, 2004},
	{15, 2233},
	{20, 2406},
	{25, 2680},
	{30, 2931},
	{35, 3189},
	{40, 3382},
	{45, 3452},
	{50, 3453},
	{55, 3454},
	{60, 3455},
	{65, 3456},
	{70, 3457},
	{75, 3486},
	{80, 3487},
	{85, 3488},
	{90, 3489},
	{95, 3501},
	{100, 3502}};
#define REAR_MAP_SIZE (sizeof(rpm_rear_map) / sizeof(struct rpm_to_pct_map))

/*
 * Mappings from temperatures recorded from sensors to fan speeds;
 * note that in some cases, we want to be able to look at offsets
 * from the CPU temperature margin rather than an absolute temperature,
 * so we use ints.
 */

struct temp_to_pct_map {
	int temp;
	unsigned speed;
};

struct temp_to_pct_map chipset_map[] = {{0, 10},
					{60, 15},
					{64, 20},
					{68, 25},
					{72, 30},
					{76, 35},
					{78, 100}};
#define CHIPSET_MAP_SIZE (sizeof(chipset_map) / sizeof(struct temp_to_pct_map))

struct temp_to_pct_map cpu_map[] =	{{0, 15},
					{10, 22},
					{20, 29},
					{30, 36},
					{40, 43},
					{50, 50},
					{54, 57},
					{58, 64},
					{62, 71},
					{66, 100}};
#define CPU_MAP_SIZE (sizeof(cpu_map) / sizeof(struct temp_to_pct_map))

struct temp_to_pct_map cpu_case_map[] =	{{0, 10},
					{30, 15},
					{40, 20},
					{50, 25},
					{54, 30},
					{58, 35},
					{62, 40},
					{66, 100}};
#define CPU_CASE_MAP_SIZE (sizeof(cpu_case_map) / sizeof(struct temp_to_pct_map))

#define FAN_FAILURE_OFFSET 30

int fan_low = WEDGE_FAN_LOW;
int fan_medium = WEDGE_FAN_MEDIUM;
int fan_high = WEDGE_FAN_HIGH;
int fan_max = WEDGE_FAN_MAX;
int total_fans = FANS;
int fan_offset = 0;

int temp_bottom = TEMP_BOTTOM;
int temp_top = TEMP_TOP;

int report_temp = REPORT_TEMP;
bool verbose = false;

int cpu2_installed = 1;

void usage() {
	fprintf(stderr,
			"fand [-v] [-l <low-pct>] [-m <medium-pct>] "
			"[-h <high-pct>]\n"
			"\t[-b <temp-bottom>] [-t <temp-top>] [-r <report-temp>]\n\n"
			"\tlow-pct defaults to %d%% fan\n"
			"\tmedium-pct defaults to %d%% fan\n"
			"\thigh-pct defaults to %d%% fan\n"
			"\ttemp-bottom defaults to %dC\n"
			"\ttemp-top defaults to %dC\n"
			"\treport-temp defaults to every %d measurements\n\n"
			"fand compensates for uServer temperature reading %d degrees low\n"
			"kill with SIGUSR1 to stop watchdog\n",
			fan_low,
			fan_medium,
			fan_high,
			EXTERNAL_TEMPS(temp_bottom),
			EXTERNAL_TEMPS(temp_top),
			report_temp,
			EXTERNAL_TEMPS(USERVER_TEMP_FUDGE));
	exit(1);
}

/* We need to open the device each time to read a value */

int read_device(const char *device, int *value) {
	FILE *fp;
	int rc;

	fp = fopen(device, "r");
	if (!fp) {
		int err = errno;

		syslog(LOG_INFO, "failed to open device %s", device);
		return err;
	}

	rc = fscanf(fp, "%d", value);
	fclose(fp);

	if (rc != 1) {
		syslog(LOG_INFO, "failed to read device %s", device);
		return ENOENT;
	} else {
		return 0;
	}
}

/* We need to open the device again each time to write a value */

int write_device(const char *device, const char *value) {
	FILE *fp;
	int rc;

	fp = fopen(device, "w");
	if (!fp) {
		int err = errno;

		syslog(LOG_INFO, "failed to open device for write %s", device);
		return err;
	}

	rc = fputs(value, fp);
	fclose(fp);

	if (rc < 0) {
		syslog(LOG_INFO, "failed to write device %s", device);
		return ENOENT;
	} else {
		return 0;
	}
}

int read_temp(const char *device, int *value) {
	char full_name[LARGEST_DEVICE_NAME + 1];

	/* We set an impossible value to check for errors */
	*value = BAD_TEMP;
	snprintf(
			full_name, LARGEST_DEVICE_NAME, "%s", device);
	return read_device(full_name, value);
}

int read_voltage(const char *device, int *value) {
	char full_name[LARGEST_DEVICE_NAME + 1];

	/* We set an impossible value to check for errors */
	*value = BAD_VOLTAGE;
	snprintf(
			full_name, LARGEST_DEVICE_NAME, "%s", device);
	return read_device(full_name, value);
}

int read_fan_value(const int fan, const char *device, int *value) {
	char device_name[LARGEST_DEVICE_NAME];
	char output_value[LARGEST_DEVICE_NAME];
	char full_name[LARGEST_DEVICE_NAME];

	snprintf(device_name, LARGEST_DEVICE_NAME, device, fan);
	snprintf(full_name, LARGEST_DEVICE_NAME, "%s/%s", PWM_DIR,device_name);
	return read_device(full_name, value);
}

int write_fan_value(const int fan, const char *device, const int value) {
	char full_name[LARGEST_DEVICE_NAME];
	char device_name[LARGEST_DEVICE_NAME];
	char output_value[LARGEST_DEVICE_NAME];

	snprintf(device_name, LARGEST_DEVICE_NAME, device, fan);
	snprintf(full_name, LARGEST_DEVICE_NAME, "%s/%s", PWM_DIR, device_name);
	snprintf(output_value, LARGEST_DEVICE_NAME, "%d", value);
	return write_device(full_name, output_value);
}

/* Return fan rpm for target fan speed percentage. */
int fan_pct_to_rpm(const struct rpm_to_pct_map *table,
		const int table_len,
		int percent) {
	int i;

	//uint pct;
	//uint rpm;

	// find index of entry with higher percentage
	for (i = 0; i < table_len; i++) {
		if (table[i].pct > percent) {
			break;
		}
	}

	// bounds check
	if (i == 0) {
		return table[i].rpm;
	} else if (i == table_len) {
		return table[i - 1].rpm;
	}
	
	// exact match
	if (table[i - 1].pct == percent) {
		return table[i - 1].rpm;
	}

	// Interpolate the right value:

	int percent_diff = table[i].pct - table[i - 1].pct;
	int rpm_diff = table[i].rpm - table[i - 1].rpm;
	int fan_diff = table[i].pct - percent;

	return table[i].rpm - (fan_diff * rpm_diff / percent_diff);
}


// deprecated
/* Return fan speed as a percentage of maximum -- not necessarily linear. */
int fan_rpm_to_pct(const struct rpm_to_pct_map *table,
		const int table_len,
		int rpm) {
	int i;

	for (i = 0; i < table_len; i++) {
		if (table[i].rpm > rpm) {
			break;
		}
	}

	/*
	 * If the fan RPM is lower than the lowest value in the table,
	 * we may have a problem -- fans can only go so slow, and it might
	 * have stopped.  In this case, we'll return an interpolated
	 * percentage, as just returning zero is even more problematic.
	 */

	if (i == 0) {
		return (rpm * table[i].pct) / table[i].rpm;
	} else if (i == table_len) { // Fell off the top?
		return table[i - 1].pct;
	}

	// Interpolate the right percentage value:

	int percent_diff = table[i].pct - table[i - 1].pct;
	int rpm_diff = table[i].rpm - table[i - 1].rpm;
	int fan_diff = table[i].rpm - rpm;

	return table[i].pct - (fan_diff * percent_diff / rpm_diff);
}

int fan_speed_okay(const int fan, const int speed, const int slop) {
	int real_fan_speed;
	int target_fan_speed;
	int real_fan;
	int okay;
	float scale;

	/*
	 * The hardware fan numbers are different from the physical order
	 * in the box, so we have to map them:
	 */

	real_fan = fan_to_rpm_map[fan];

	real_fan_speed = 0;
	read_fan_value(real_fan, FAN_READ_RPM_FORMAT, &real_fan_speed);

	scale = fan_scale_speed_map[fan];
	if (fan < 2) {
		target_fan_speed = (int)(((float)fan_pct_to_rpm(rpm_front_map, FRONT_MAP_SIZE, speed)) / scale);
	} else {
		target_fan_speed = (int)(((float)fan_pct_to_rpm(rpm_rear_map, REAR_MAP_SIZE, speed)) / scale);
	}

	okay = ((abs(real_fan_speed - target_fan_speed) * 100 / target_fan_speed) < slop);

#if defined(CONFIG_SLOPPY_FANS)
	// Don't alert if fan is spinning faster than expected...
	if (!okay && (real_fan_speed > target_fan_speed)) {
		okay = 1;
	}
#endif

	if (!okay || verbose) {
		syslog(!okay ? LOG_WARNING : LOG_INFO,
				"fan %d %d RPM, expected %d",
				fan,
				real_fan_speed,
				target_fan_speed);
	}

	return okay;
}

int averaged_pwm_values[2] = {0, 0};

/* Set fan speed as a percentage */

int write_fan_speed(const int fan, const int value) {
	/*
	 * The hardware fan numbers for pwm control are different from
	 * both the physical order in the box, and the mapping for reading
	 * the RPMs per fan, above.
	 */

	int real_fan = fan_to_pwm_map[fan];

	if (value == 0) {
		return write_fan_value(real_fan, "pwm%d_enable", 0);
	} else {
		int unit = (value * PWM_UNIT_MAX) / 100;
		int status;

		// The KGPE-D16 / KCMA-D8 only has two fan outputs; one controls all 4 pin fans,
		// and the other controls all 3 pin fans.  Average the requested PWM values
		// as a middle-of-the-road thermal strategy...
		if (fan == 0) {
			if (cpu2_installed) {
				averaged_pwm_values[0] = unit;
				return 0;
			}
		}
		else if (fan == 1) {
			if (cpu2_installed) {
				averaged_pwm_values[0] += unit;
				averaged_pwm_values[0] /= 2;
				unit = averaged_pwm_values[0];
			}
			else {
				return 0;
			}
		}
		else if (fan == 2) {
			averaged_pwm_values[1] = unit;
			return 0;
		}
		else if ((fan > 2) && (fan < 7)) {
			averaged_pwm_values[1] += unit;
			return 0;
		}
		else if (fan == 7) {
			averaged_pwm_values[1] += unit;
			averaged_pwm_values[1] /= 6;
			unit = averaged_pwm_values[1];
		}

		if ((status = write_fan_value(real_fan, "pwm%d_enable", 1)) != 0) {
			return status;
		}
		if ((status = write_fan_value(real_fan, "pwm%d", unit)) != 0) {
			return status;
		}
	}
}

int temp_to_fan_speed(int temp, struct temp_to_pct_map *map, int map_size) {
	int i = map_size - 1;

	while (i > 0 && temp < map[i].temp) {
		--i;
	}
	return map[i].speed;
}

/* Set up fan LEDs */

int write_fan_led(const int fan, const char *color) {
	return 0;
}

int server_shutdown(const char *why) {
	int fan;
	for (fan = 0; fan < total_fans; fan++) {
		write_fan_speed(fan + fan_offset, fan_max);
	}

	syslog(LOG_EMERG, "Shutting down:  %s", why);
	write_device(MAIN_POWER_OFF_DIRECTION, "out");
	write_device(MAIN_POWER_OFF_CTL, "0");
	sleep(1);
	write_device(MAIN_POWER_OFF_CTL, "1");

}

/* Gracefully shut down on receipt of a signal */

void fand_interrupt(int sig)
{
	int fan;
	for (fan = 0; fan < total_fans; fan++) {
		write_fan_speed(fan + fan_offset, fan_max);
	}

	syslog(LOG_WARNING, "Shutting down fand on signal %s", strsignal(sig));
	if (sig == SIGUSR1) {
		stop_watchdog();
	}
	exit(3);
}

int main(int argc, char **argv) {
	/* Sensor values */

	int chipset_temp;
	int cpu1_temp;
	int cpu2_temp;
	int cpu2_vcore_mv;

	int fan_speed = fan_max;
	int bad_reads = 0;
	int fan_failure = 0;
	int fan_speed_changes = 0;
	int old_speed;

	int cpu_fan_speed = fan_max;
	int chassis_fan_speed = fan_max;
	int cpu_fan_speed_changes = 0;
	int chassis_fan_speed_changes = 0;
	int cpu_old_speed = fan_max;
	int chassis_old_speed = fan_max;

	int fan_bad[FANS];
	int fan;

	unsigned log_count = 0; // How many times have we logged our temps?
	int opt;
	int prev_fans_bad = 0;

	struct sigaction sa;

	sa.sa_handler = fand_interrupt;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);

	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);

	// Start writing to syslog as early as possible for diag purposes.

	openlog("fand", LOG_CONS, LOG_DAEMON);

	while ((opt = getopt(argc, argv, "l:m:h:b:t:r:v")) != -1) {
		switch (opt) {
			case 'l':
				fan_low = atoi(optarg);
				break;
			case 'm':
				fan_medium = atoi(optarg);
				break;
			case 'h':
				fan_high = atoi(optarg);
				break;
			case 'b':
				temp_bottom = INTERNAL_TEMPS(atoi(optarg));
				break;
			case 't':
				temp_top = INTERNAL_TEMPS(atoi(optarg));
				break;
			case 'r':
				report_temp = atoi(optarg);
				break;
			case 'v':
				verbose = true;
				break;
			default:
				usage();
				break;
		}
	}

	if (optind > argc) {
		usage();
	}

	if (temp_bottom > temp_top) {
		fprintf(stderr,
				"Should temp-bottom (%d) be higher than "
				"temp-top (%d)?  Starting anyway.\n",
				EXTERNAL_TEMPS(temp_bottom),
				EXTERNAL_TEMPS(temp_top));
	}

	if (fan_low > fan_medium || fan_low > fan_high || fan_medium > fan_high) {
		fprintf(stderr,
				"fan RPMs not strictly increasing "
				"-- %d, %d, %d, starting anyway\n",
				fan_low,
				fan_medium,
				fan_high);
	}

	daemon(1, 0);

	if (verbose) {
		syslog(LOG_DEBUG, "Starting up;  system should have %d fans.",
				total_fans);
	}

	for (fan = 0; fan < total_fans; fan++) {
		if (!cpu2_installed && (fan == 1)) {
			continue;
		}
		fan_bad[fan] = 0;
		write_fan_speed(fan + fan_offset, fan_speed);
		write_fan_led(fan + fan_offset, FAN_LED_BLUE);
	}

	/* Start watchdog in manual mode */
	start_watchdog(0);

	/* Set watchdog to persistent mode so timer expiry will happen independent
	 * of this process's liveliness. */
	set_persistent_watchdog(WATCHDOG_SET_PERSISTENT);

	sleep(5);  /* Give the fans time to come up to speed */

	while (1) {
		int max_temp;
		old_speed = fan_speed;
		cpu_old_speed = cpu_fan_speed;
		chassis_old_speed = chassis_fan_speed;

		/* Read sensors */

		read_temp(CHIPSET_TEMP_DEVICE, &chipset_temp);
		read_temp(CPU1_TEMP_DEVICE, &cpu1_temp);
		read_temp(CPU2_TEMP_DEVICE, &cpu2_temp);
		read_voltage(CPU2_VCORE_DEVICE, &cpu2_vcore_mv);

		/*
		 * uServer can be powered down, but all of the rest of the sensors
		 * should be readable at any time.
		 */

		if ((chipset_temp == BAD_TEMP) || (cpu1_temp == BAD_TEMP) ||
				(cpu2_temp == BAD_TEMP) || (cpu2_vcore_mv == BAD_VOLTAGE)) {
			bad_reads++;
		}
		else {
			bad_reads = 0;	// Only care about continuous bad reads
		}

		if (cpu2_vcore_mv == 0) {
			cpu2_installed = 0;
		}
		else {
			cpu2_installed = 1;
		}

		if (bad_reads > BAD_READ_THRESHOLD) {
			server_shutdown("Some sensors couldn't be read");
			bad_reads = 0;
			kick_watchdog();
			continue;
		}

		if (log_count++ % report_temp == 0) {
			syslog(LOG_DEBUG,
					"Temp chipset %f, CPU1 %f, CPU2 %f, "
					"CPU fan speed %d, CPU speed changes %d "
					"chassis fan speed %d, chassis speed changes %d",
					EXTERNAL_TEMPS((float)chipset_temp),
					EXTERNAL_TEMPS((float)cpu1_temp),
					EXTERNAL_TEMPS((float)cpu2_temp),
					cpu_fan_speed,
					cpu_fan_speed_changes,
					chassis_fan_speed,
					chassis_fan_speed_changes);
		}

		/* Protection heuristics */

		if (chipset_temp > CHIPSET_LIMIT) {
			server_shutdown("Chipset temp limit reached");
			bad_reads = 0;
			kick_watchdog();
			continue;
		}

		if (cpu1_temp > CPU_LIMIT) {
			server_shutdown("CPU1 temp limit reached");
			bad_reads = 0;
			kick_watchdog();
			continue;
		}

		if (cpu2_installed) {
			if (cpu2_temp > CPU_LIMIT) {
				server_shutdown("CPU2 temp limit reached");
				bad_reads = 0;
				kick_watchdog();
				continue;
			}
		}

		/*
		 * Calculate change needed -- we should eventually
		 * do something more sophisticated, like PID.
		 *
		 * We should use the intake temperature to adjust this
		 * as well.
		 */

		int max_cpu_temp = cpu1_temp + USERVER_TEMP_FUDGE;

		if (cpu2_installed) {
			if (cpu2_temp + USERVER_TEMP_FUDGE > max_cpu_temp) {
				max_cpu_temp = cpu2_temp + USERVER_TEMP_FUDGE;
			}
		}

		int max_cpu_temp_slop = max_cpu_temp + COOLDOWN_SLOP;
		int chipset_temp_slop = chipset_temp + COOLDOWN_SLOP;

		int chassis_speed = temp_to_fan_speed(EXTERNAL_TEMPS(chipset_temp), chipset_map,
				CHIPSET_MAP_SIZE);
		int chassis_slop = temp_to_fan_speed(EXTERNAL_TEMPS(chipset_temp_slop), chipset_map,
				CHIPSET_MAP_SIZE);
		int cpu_speed = temp_to_fan_speed(EXTERNAL_TEMPS(max_cpu_temp), cpu_map, CPU_MAP_SIZE);
		int cpu_slop = temp_to_fan_speed(EXTERNAL_TEMPS(max_cpu_temp_slop), cpu_map, CPU_MAP_SIZE);
		int cpu_case_speed = temp_to_fan_speed(EXTERNAL_TEMPS(max_cpu_temp), cpu_case_map, CPU_CASE_MAP_SIZE);
		int cpu_case_slop = temp_to_fan_speed(EXTERNAL_TEMPS(max_cpu_temp_slop), cpu_case_map, CPU_CASE_MAP_SIZE);
		
		if(cpu_case_speed > chassis_speed) {
			chassis_speed = cpu_case_speed;
		}
		if(cpu_case_slop > chassis_slop) {
			chassis_slop = cpu_case_slop;
		}

		if (cpu_fan_speed == fan_max && fan_failure != 0) {
			/* Don't change a thing */
		} else {
			if(cpu_speed > cpu_old_speed) {
				cpu_fan_speed = cpu_speed;
			} else {
				if(cpu_slop < cpu_old_speed) {
					cpu_fan_speed = cpu_speed;
				}
			}
		}
		if (chassis_fan_speed == fan_max && fan_failure != 0) {
			/* Don't change a thing */
		} else {
			if(chassis_speed > chassis_old_speed) {
				chassis_fan_speed = chassis_speed;
			} else {
				if(chassis_slop < chassis_old_speed) {
					chassis_fan_speed = chassis_speed;
				}
			}
		}

		/*
		 * Update fans only if there are no failed ones. If any fans failed
		 * earlier, all remaining fans should continue to run at max speed.
		 */

		if (fan_failure == 0 && cpu_fan_speed != cpu_old_speed) {
			syslog(LOG_NOTICE,
					"CPU fan speed changing from %d to %d",
					cpu_old_speed,
					cpu_fan_speed);
			cpu_fan_speed_changes++;
		}
		if (fan_failure == 0 && chassis_fan_speed != chassis_old_speed) {
			syslog(LOG_NOTICE,
					"Chassis fan speed changing from %d to %d",
					chassis_old_speed,
					chassis_fan_speed);
			chassis_fan_speed_changes++;
		}
		for (fan = 0; fan < 2; fan++) {
			if (!cpu2_installed && (fan == 1)) {
				continue;
			}
			write_fan_speed(fan + fan_offset, cpu_fan_speed);
		}
		for (fan = 2; fan < total_fans; fan++) {
			write_fan_speed(fan + fan_offset, chassis_fan_speed);
		}

		/*
		 * Wait for some change.  Typical I2C temperature sensors
		 * only provide a new value every second and a half, so
		 * checking again more quickly than that is a waste.
		 *
		 * We also have to wait for the fan changes to take effect
		 * before measuring them.
		 */

		sleep(5);

		/* Check fan RPMs */

		for (fan = 0; fan < total_fans; fan++) {
			if (!cpu2_installed && (fan == 1)) {
				continue;
			}
			if (fan_population_map[fan] == 0) {
				continue;
			}
			/*
			 * Make sure that we're within some percentage
			 * of the requested speed.
			 */
			int desired_fan_speed = cpu_fan_speed;
			if (fan > 1) {
				desired_fan_speed = chassis_fan_speed;
			}
			if (fan_speed_okay(fan + fan_offset, desired_fan_speed, FAN_FAILURE_OFFSET)) {
				if (fan_bad[fan] > FAN_FAILURE_THRESHOLD) {
					write_fan_led(fan + fan_offset, FAN_LED_BLUE);
					syslog(LOG_CRIT,
							"Fan %d has recovered",
							fan);
				}
				fan_bad[fan] = 0;
			} else {
				fan_bad[fan]++;
			}
		}

		fan_failure = 0;
		for (fan = 0; fan < total_fans; fan++) {
			if (!cpu2_installed && (fan == 1)) {
				continue;
			}
			if (fan_population_map[fan] == 0) {
				continue;
			}
			if (fan_bad[fan] > FAN_FAILURE_THRESHOLD) {
				fan_failure++;
				write_fan_led(fan + fan_offset, FAN_LED_RED);
			}
		}

		if (fan_failure > 0) {
			if (prev_fans_bad != fan_failure) {
				syslog(LOG_CRIT, "%d fans failed", fan_failure);
			}

			/*
			 * If fans are bad, we need to blast all of the
			 * fans at 100%;  we don't bother to turn off
			 * the bad fans, in case they are all that is left.
			 *
			 * Note that we have a temporary bug with setting fans to
			 * 100% so we only do fan_max = 99%.
			 */

			fan_speed = fan_max;
			cpu_fan_speed = fan_max;
			chassis_fan_speed = fan_max;
			for (fan = 0; fan < total_fans; fan++) {
				if (!cpu2_installed && (fan == 1)) {
					continue;
				}
				if (fan_population_map[fan] == 0) {
					continue;
				}
				write_fan_speed(fan + fan_offset, fan_max);
			}

			/*
			 * Fans can be hot swapped and replaced; in which case the fan daemon
			 * will automatically detect the new fan and (assuming the new fan isn't
			 * itself faulty), automatically readjust the speeds for all fans down
			 * to a more suitable rpm. The fan daemon does not need to be restarted.
			 */
		}

		/* Suppress multiple warnings for similar number of fan failures. */
		prev_fans_bad = fan_failure;

		/* if everything is fine, restart the watchdog countdown. If this process
		 * is terminated, the persistent watchdog setting will cause the system
		 * to reboot after the watchdog timeout. */
		kick_watchdog();
	}
}

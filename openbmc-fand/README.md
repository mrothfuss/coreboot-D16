# fand.cpp replacement for OpenBMC

The following variables must be set to match your hardware setup:

```c
fan_scale_speed_map[]
fan_population_map[]
fan_to_pwm_map[]
COOLDOWN_SLOP
CHIPSET_LIMIT
CPU_LIMIT
rpm_front_map[]
rpm_rear_map[]
chipset_map[]
cpu_map[]
cpu_case_map[]
FAN_FAILURE_OFFSET
```

This is setup to use the 4pin fan zone for CPU fans and 3pin fan zone for CASE fans. If you want to use a 4pin fan as a 3pin fan, you must use a 4pin-to-3pin adapter. This fan controller takes a fair bit of calibration to be useful. lm-sensors can be used to set/measure fan speeds in linux to determine values for rpm_front_map[], rpm_rear_map[], and fan_scale_speed_map[].

CPU temperature sets both CPU and CASE fan speeds

CHIPSET temperature sets only CASE fan speeds

Higher fan speeds override lower fan speeds

Variable explanations:
```c
fan_population_map[]  // needs to match the fans that are connected
fan_scale_speed_map[] // is used to account for RPM variation between fans within each fan zone
fan_to_pwm_map[]      // is used to assign fans to 4pin/3pin zones (must match the fan setup and jumpers)
COOLDOWN_SLOP         // sets the required temperature drop before the fans switch to a lower speed
CHIPSET_LIMIT         // sets the emergency power off point for chipset thermals
CPU_LIMIT             // sets the emergency power off point for cpu thermals
rpm_front_map[]       // translates fan % to fan RPM for CPU fans
rpm_rear_map[]        // translates fan % to fan RPM for CASE fans
chipset_map[]         // translates chipset temperatures to case fan %
cpu_map[]             // translates cpu temperatures to cpu fan %
cpu_case_map[]        // translates cpu temperatures to case fan %
FAN_FAILURE_OFFSET    // sets the error% margin in fan speeds for fan failure detection (fans switch to 100% when a failture is detected)
```

/* Copyright 2021 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define DT_DRV_COMPAT ite_it8xxx2_cros_flash

#include <drivers/cros_flash.h>
#include <drivers/flash.h>
#include <kernel.h>
#include <logging/log.h>
#include <soc.h>

#include "flash.h"
#include "host_command.h"
#include "system.h"

LOG_MODULE_REGISTER(cros_flash, LOG_LEVEL_ERR);

/* Device data */
struct cros_flash_it8xxx2_data {
	bool stuck_locked;
	bool inconsistent_locked;
	bool all_protected;
};

/* Driver convenience defines */
#define DRV_DATA(dev) ((struct cros_flash_it8xxx2_data *)(dev)->data)

#define FLASH_DEV_NAME DT_CHOSEN_ZEPHYR_FLASH_CONTROLLER_LABEL
static const struct device *flash_controller;

#define FWP_REG(bank) (bank / 8)
#define FWP_MASK(bank) (1 << (bank % 8))

enum flash_wp_interface {
	FLASH_WP_HOST = 0x01,
	FLASH_WP_DBGR = 0x02,
	FLASH_WP_EC = 0x04,
};

enum flash_wp_status {
	FLASH_WP_STATUS_PROTECT_RO = EC_FLASH_PROTECT_RO_NOW,
	FLASH_WP_STATUS_PROTECT_ALL = EC_FLASH_PROTECT_ALL_NOW,
};

/**
 * Protect flash banks until reboot.
 *
 * @param start_bank    Start bank to protect
 * @param bank_count    Number of banks to protect
 */
static void flash_protect_banks(int start_bank, int bank_count,
				enum flash_wp_interface wp_if)
{
	int bank;

	for (bank = start_bank; bank < start_bank + bank_count; bank++) {
		if (wp_if & FLASH_WP_EC)
			IT83XX_GCTRL_EWPR0PFEC(FWP_REG(bank)) |= FWP_MASK(bank);
		if (wp_if & FLASH_WP_HOST)
			IT83XX_GCTRL_EWPR0PFH(FWP_REG(bank)) |= FWP_MASK(bank);
		if (wp_if & FLASH_WP_DBGR)
			IT83XX_GCTRL_EWPR0PFD(FWP_REG(bank)) |= FWP_MASK(bank);
	}
}

static enum flash_wp_status flash_check_wp(void)
{
	enum flash_wp_status wp_status;
	int all_bank_count, bank;

	all_bank_count = CONFIG_FLASH_SIZE_BYTES / CONFIG_FLASH_BANK_SIZE;

	for (bank = 0; bank < all_bank_count; bank++) {
		if (!(IT83XX_GCTRL_EWPR0PFEC(FWP_REG(bank)) & FWP_MASK(bank)))
			break;
	}

	if (bank == WP_BANK_COUNT)
		wp_status = FLASH_WP_STATUS_PROTECT_RO;
	else if (bank == (WP_BANK_COUNT + PSTATE_BANK_COUNT))
		wp_status = FLASH_WP_STATUS_PROTECT_RO;
	else if (bank == all_bank_count)
		wp_status = FLASH_WP_STATUS_PROTECT_ALL;
	else
		wp_status = 0;

	return wp_status;
}

/* cros ec flash api functions */
static int cros_flash_it8xxx2_init(const struct device *dev)
{
	struct cros_flash_it8xxx2_data *const data = DRV_DATA(dev);
	int32_t reset_flags, prot_flags, unwanted_prot_flags;

	reset_flags = system_get_reset_flags();
	prot_flags = crec_flash_get_protect();
	unwanted_prot_flags = EC_FLASH_PROTECT_ALL_NOW |
		EC_FLASH_PROTECT_ERROR_INCONSISTENT;

	/*
	 * If we have already jumped between images, an earlier image could
	 * have applied write protection.  Nothing additional needs to be done.
	 */
	if (reset_flags & EC_RESET_FLAG_SYSJUMP)
		return EC_SUCCESS;

	if (prot_flags & EC_FLASH_PROTECT_GPIO_ASSERTED) {
		/* Protect the entire flash of host interface */
		flash_protect_banks(0,
			CONFIG_FLASH_SIZE_BYTES / CONFIG_FLASH_BANK_SIZE,
			FLASH_WP_HOST);
		/* Protect the entire flash of DBGR interface */
		flash_protect_banks(0,
			CONFIG_FLASH_SIZE_BYTES / CONFIG_FLASH_BANK_SIZE,
			FLASH_WP_DBGR);
		/*
		 * Write protect is asserted.  If we want RO flash protected,
		 * protect it now.
		 */
		if ((prot_flags & EC_FLASH_PROTECT_RO_AT_BOOT) &&
		    !(prot_flags & EC_FLASH_PROTECT_RO_NOW)) {
			int rv = crec_flash_set_protect(EC_FLASH_PROTECT_RO_NOW,
						   EC_FLASH_PROTECT_RO_NOW);
			if (rv)
				return rv;

			/* Re-read flags */
			prot_flags = crec_flash_get_protect();
		}
	} else {
		/* Don't want RO flash protected */
		unwanted_prot_flags |= EC_FLASH_PROTECT_RO_NOW;
	}

	/* If there are no unwanted flags, done */
	if (!(prot_flags & unwanted_prot_flags))
		return EC_SUCCESS;

	/*
	 * If the last reboot was a power-on reset, it should have cleared
	 * write-protect.  If it didn't, then the flash write protect registers
	 * have been permanently committed and we can't fix that.
	 */
	if (reset_flags & EC_RESET_FLAG_POWER_ON) {
		data->stuck_locked = 1;
		return EC_ERROR_ACCESS_DENIED;
	} else {
		/*
		 * Set inconsistent flag, because there is no software
		 * reset can clear write-protect.
		 */
		data->inconsistent_locked = 1;
		return EC_ERROR_ACCESS_DENIED;
	}

	/* That doesn't return, so if we're still here that's an error */
	return EC_ERROR_UNKNOWN;
}

static int cros_flash_it8xxx2_read(const struct device *dev, int offset,
				   int size, char *dst_data)
{
	ARG_UNUSED(dev);

	return flash_read(flash_controller, offset, dst_data, size);
}

static int cros_flash_it8xxx2_write(const struct device *dev, int offset,
				    int size, const char *src_data)
{
	struct cros_flash_it8xxx2_data *const data = DRV_DATA(dev);

	if (data->all_protected) {
		return -EACCES;
	}

	return flash_write(flash_controller, offset, src_data, size);
}

static int cros_flash_it8xxx2_erase(const struct device *dev, int offset,
				    int size)
{
	struct cros_flash_it8xxx2_data *const data = DRV_DATA(dev);

	if (data->all_protected) {
		return -EACCES;
	}

	return flash_erase(flash_controller, offset, size);
}

static int cros_flash_it8xxx2_get_protect(const struct device *dev, int bank)
{
	ARG_UNUSED(dev);

	return IT83XX_GCTRL_EWPR0PFEC(FWP_REG(bank)) & FWP_MASK(bank);
}

static uint32_t cros_flash_it8xxx2_get_protect_flags(const struct device *dev)
{
	struct cros_flash_it8xxx2_data *const data = DRV_DATA(dev);
	uint32_t flags = 0;

	flags |= flash_check_wp();

	if (data->all_protected)
		flags |= EC_FLASH_PROTECT_ALL_NOW;

	/* Check if blocks were stuck locked at pre-init */
	if (data->stuck_locked)
		flags |= EC_FLASH_PROTECT_ERROR_STUCK;

	/* Check if flash protection is in inconsistent state at pre-init */
	if (data->inconsistent_locked)
		flags |= EC_FLASH_PROTECT_ERROR_INCONSISTENT;

	return flags;
}

static int cros_flash_it8xxx2_protect_at_boot(const struct device *dev,
					      uint32_t new_flags)
{
	return -ENOTSUP;
}

static int cros_flash_it8xxx2_protect_now(const struct device *dev, int all)
{
	struct cros_flash_it8xxx2_data *const data = DRV_DATA(dev);

	if (all) {
		/* Protect the entire flash */
		flash_protect_banks(0,
			CONFIG_FLASH_SIZE_BYTES / CONFIG_FLASH_BANK_SIZE,
			FLASH_WP_EC);
		data->all_protected = 1;
	} else {
		/* Protect the read-only section and persistent state */
		flash_protect_banks(WP_BANK_OFFSET,
			WP_BANK_COUNT, FLASH_WP_EC);
#ifdef PSTATE_BANK
		flash_protect_banks(PSTATE_BANK,
			PSTATE_BANK_COUNT, FLASH_WP_EC);
#endif
	}

	return EC_SUCCESS;
}

/* cros ec flash driver registration */
static const struct cros_flash_driver_api cros_flash_it8xxx2_driver_api = {
	.init = cros_flash_it8xxx2_init,
	.physical_read = cros_flash_it8xxx2_read,
	.physical_write = cros_flash_it8xxx2_write,
	.physical_erase = cros_flash_it8xxx2_erase,
	.physical_get_protect = cros_flash_it8xxx2_get_protect,
	.physical_get_protect_flags = cros_flash_it8xxx2_get_protect_flags,
	.physical_protect_at_boot = cros_flash_it8xxx2_protect_at_boot,
	.physical_protect_now = cros_flash_it8xxx2_protect_now,
};

static int flash_it8xxx2_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	flash_controller = device_get_binding(FLASH_DEV_NAME);
	if (!flash_controller) {
		LOG_ERR("Fail to find %s", FLASH_DEV_NAME);
		return -ENODEV;
	}

	return 0;
}

static struct cros_flash_it8xxx2_data cros_flash_data;

DEVICE_DEFINE(cros_flash_it8xxx2_0, DT_INST_LABEL(0), flash_it8xxx2_init, NULL,
	      &cros_flash_data, NULL, PRE_KERNEL_1,
	      CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
	      &cros_flash_it8xxx2_driver_api);

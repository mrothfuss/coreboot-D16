/* SPDX-License-Identifier: GPL-2.0-only */

// Use simple device model for this file even in ramstage
#define __SIMPLE_DEVICE__

#include <arch/io.h>
#include <device/pci_ops.h>
#include <reset.h>
#include <southbridge/amd/common/reset.h>

#define HT_INIT_CONTROL		0x6C
#define HTIC_BIOSR_Detect	(1<<5)

#if CONFIG_MAX_PHYSICAL_CPUS > 32
#define NODE_PCI(x, fn)        ((x < 32)?(PCI_DEV(CONFIG_CBB,(CONFIG_CDB+x),fn)):(PCI_DEV((CONFIG_CBB-1),(CONFIG_CDB+x-32),fn)))
#else
#define NODE_PCI(x, fn)        PCI_DEV(CONFIG_CBB,(CONFIG_CDB+x),fn)
#endif

static void set_bios_reset(void)
{
	u32 nodes;
	u32 htic;
	pci_devfn_t dev;
	int i;

	nodes = ((pci_read_config32(PCI_DEV(CONFIG_CBB, CONFIG_CDB, 0), 0x60) >> 4) & 7) + 1;
	for (i = 0; i < nodes; i++) {
		dev = NODE_PCI(i, 0);
		htic = pci_read_config32(dev, HT_INIT_CONTROL);
		htic &= ~HTIC_BIOSR_Detect;
		pci_write_config32(dev, HT_INIT_CONTROL, htic);
	}
}

void do_board_reset(void)
{
	set_bios_reset();

	/* Try rebooting through port 0xcf9 */
	/* Actually it is not a real hard_reset
	 * --- it only reset coherent link table, but not reset link freq and width
	 */
	outb((0 << 3) | (0 << 2) | (1 << 1), 0xcf9);
	outb((0 << 3) | (1 << 2) | (1 << 1), 0xcf9);
}

void do_soft_reset(void)
{
	set_bios_reset();
	/* link reset */
	outb(0x06, 0x0cf9);
}

/* SPDX-License-Identifier: GPL-2.0-only */

#include <cpu/amd/common/nums.h>
#include <device/pci_ops.h>
#include <northbridge/amd/amdfam10/amdfam10.h>

#include "raminit.h"

static void set_htic_bit(u8 i, u32 val, u8 bit)
{
	u32 dword;
	dword = pci_read_config32(NODE_PCI(i, 0), HT_INIT_CONTROL);
	dword &= ~(1 << bit);
	dword |= ((val & 1) << bit);
	pci_write_config32(NODE_PCI(i, 0), HT_INIT_CONTROL, dword);
}

void fill_mem_ctrl(u32 controllers, struct mem_controller *ctrl_a, const u8 *spd_addr)
{
	int i;
	int j;
	int index = 0;
	struct mem_controller *ctrl;
	for (i = 0; i < controllers; i++) {
		ctrl = &ctrl_a[i];
		ctrl->node_id = i;
		ctrl->f0 = NODE_PCI(i, 0);
		ctrl->f1 = NODE_PCI(i, 1);
		ctrl->f2 = NODE_PCI(i, 2);
		ctrl->f3 = NODE_PCI(i, 3);
		ctrl->f4 = NODE_PCI(i, 4);
		ctrl->f5 = NODE_PCI(i, 5);

		if (spd_addr == (void *)0) continue;

		ctrl->spd_switch_addr = spd_addr[index++];

		for (j = 0; j < 8; j++) {
			ctrl->spd_addr[j] = spd_addr[index++];

		}
	}
}

void set_sysinfo_in_ram(u32 val)
{
	set_htic_bit(0, val, 9);
}

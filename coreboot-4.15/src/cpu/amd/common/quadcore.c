/* SPDX-License-Identifier: GPL-2.0-only */

#include <console/console.h>
#include <device/device.h>
#include <device/pci_ops.h>
#include <types.h>
#include "common.h"
#include "nums.h"

/* Called by amd_siblings (ramstage) as well */
struct node_core_id get_node_core_id(u32 nb_cfg_54)
{
	struct node_core_id id;
	uint8_t apicid;
	uint8_t fam15h = 0;
	uint8_t rev_gte_d = 0;
	uint8_t dual_node = 0;
	uint32_t f3xe8;
	uint32_t family;
	uint32_t model;

	f3xe8 = pci_read_config32(NODE_PCI(0, 3), 0xe8);

	family = model = cpuid_eax(0x80000001);
	model = ((model & 0xf0000) >> 12) | ((model & 0xf0) >> 4);
	family = ((family & 0xf00000) >> 16) | ((family & 0xf00) >> 8);

	if (family >= 0x6f) {
		/* Family 15h or later */
		fam15h = 1;
		nb_cfg_54 = 1;
	}

	if ((model >= 0x8) || fam15h)
		/* Revision D or later */
		rev_gte_d = 1;

	if (rev_gte_d)
		 /* Check for dual node capability */
		if (f3xe8 & 0x20000000)
			dual_node = 1;

	/* Get the apicid via cpuid(1) ebx[31:24]
	 * The apicid format varies based on processor revision
	 */
	apicid = (cpuid_ebx(1) >> 24) & 0xff;
	if (nb_cfg_54) {
		if (fam15h && dual_node) {
			id.coreid = apicid & 0x1f;
			id.nodeid = (apicid & 0x60) >> 5;
		} else if (fam15h && !dual_node) {
			id.coreid = apicid & 0xf;
			id.nodeid = (apicid & 0x70) >> 4;
		} else if (rev_gte_d && dual_node) {
			id.coreid = apicid & 0xf;
			id.nodeid = (apicid & 0x30) >> 4;
		} else if (rev_gte_d && !dual_node) {
			id.coreid = apicid & 0x7;
			id.nodeid = (apicid & 0x38) >> 3;
		} else {
			id.coreid = apicid & 0x3;
			id.nodeid = (apicid & 0x1c) >> 2;
		}
	} else {
		if (rev_gte_d && dual_node) {
			id.coreid = (apicid & 0xf0) >> 4;
			id.nodeid = apicid & 0x3;
		} else if (rev_gte_d && !dual_node) {
			id.coreid = (apicid & 0xe0) >> 5;
			id.nodeid = apicid & 0x7;
		} else {
			id.coreid = (apicid & 0x60) >> 5;
			id.nodeid = apicid & 0x7;
		}
	}
	if (fam15h && dual_node) {
		/* coreboot expects each separate processor die to be on a
		 * different nodeid.
		 * Since the code above returns nodeid 0 even on
		 * internal node 1 some fixup is needed...
		 */
		uint32_t f5x84;
		uint8_t core_count;

		f5x84 = pci_read_config32(NODE_PCI(0, 5), 0x84);

		core_count = (f5x84 & 0xff) + 1;
		id.nodeid = id.nodeid * 2;
		if (id.coreid >= core_count) {
			id.nodeid += 1;
			id.coreid = id.coreid - core_count;
		}
	} else if (rev_gte_d && dual_node) {
		/* coreboot expects each separate processor die to be on a
		 * different nodeid.
		 * Since the code above returns nodeid 0 even on
		 * internal node 1 some fixup is needed...
		 */
		uint8_t core_count = (((f3xe8 & 0x00008000) >> 13) |
				     ((f3xe8 & 0x00003000) >> 12)) + 1;

		id.nodeid = id.nodeid * 2;
		if (id.coreid >= core_count) {
			id.nodeid += 1;
			id.coreid = id.coreid - core_count;
		}
	}

	return id;
}

struct node_core_id get_node_core_id_x(void)
{
	return get_node_core_id(read_nb_cfg_54());
}

uint32_t get_boot_apic_id(uint8_t node, uint32_t core)
{
	uint32_t ap_apicid;
	uint32_t nb_cfg_54;
	uint32_t siblings;
	uint32_t cores_found;
	uint8_t fam15h = 0;
	uint8_t rev_gte_d = 0;
	uint8_t dual_node = 0;
	uint32_t f3xe8;
	uint32_t family;
	uint32_t model;

	uint32_t apic_id_core_id_size;

	/* Assume that all node are same stepping, otherwise we can use use
	   nb_cfg_54 from bsp for all nodes */
	nb_cfg_54 = read_nb_cfg_54();
	f3xe8 = pci_read_config32(NODE_PCI(0, 3), 0xe8);

	family = model = cpuid_eax(0x80000001);
	model = ((model & 0xf0000) >> 12) | ((model & 0xf0) >> 4);
	family = ((family & 0xf00000) >> 16) | ((family & 0xf00) >> 8);

	if (family >= 0x6f) {
		/* Family 15h or later */
		fam15h = 1;
		nb_cfg_54 = 1;
	}

	if ((model >= 0x8) || fam15h)
		/* Revision D or later */
		rev_gte_d = 1;

	if (rev_gte_d)
		 /* Check for dual node capability */
		if (f3xe8 & 0x20000000)
			dual_node = 1;

	apic_id_core_id_size = (cpuid_ecx(0x80000008) >> 12 & 0xf);
	if (apic_id_core_id_size) {
		siblings = ((1 << apic_id_core_id_size) - 1);
	} else {
		siblings = 3;	//quad core
	}

	cores_found = get_core_num_in_bsp(node);
	if (siblings > cores_found)
		siblings = cores_found;

	if (dual_node) {
		ap_apicid = 0;
		if (fam15h) {
			ap_apicid |= ((node >> 1) & 0x3) << 5;			/* Node ID */
			ap_apicid |= ((node & 0x1) * (siblings + 1)) + core;	/* Core ID */
		} else {
			if (nb_cfg_54) {
				ap_apicid |= ((node >> 1) & 0x3) << 4;			/* Node ID */
				ap_apicid |= ((node & 0x1) * (siblings + 1)) + core;	/* Core ID */
			} else {
				ap_apicid |= node & 0x3;					/* Node ID */
				ap_apicid |= (((node & 0x1) * (siblings + 1)) + core) << 4;	/* Core ID */
			}
		}
	} else {
		if (fam15h) {
			ap_apicid = 0;
			ap_apicid |= (node & 0x7) << 4;	/* Node ID */
			ap_apicid |= core & 0xf;	/* Core ID */
		} else {
			ap_apicid = node * (nb_cfg_54 ? (siblings + 1) : 1) +
					core * (nb_cfg_54 ? 1 : 64);
		}
	}

	printk(BIOS_DEBUG, "%s: using %d as APIC ID for node %d, core %d\n", __func__, ap_apicid, node, core);

	return ap_apicid;
}

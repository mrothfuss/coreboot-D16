/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/io.h>
#include <stdint.h>
#include <console/console.h>
#include <device/pci_ops.h>
#include <northbridge/amd/amdfam10/raminit.h>
#include <northbridge/amd/amdfam10/amdfam10.h>

#define RES_DEBUG 0

void setup_resource_map(const u32 *register_values, u32 max)
{
	u32 i;

	for (i = 0; i < max; i += 3) {
		pci_devfn_t dev;
		u32 where;
		u32 reg;

		dev = register_values[i] & ~0xfff;
		where = register_values[i] & 0xfff;
		reg = pci_read_config32(dev, where);
		reg &= register_values[i+1];
		reg |= register_values[i+2];
		pci_write_config32(dev, where, reg);
	}
}


void setup_resource_map_offset(const u32 *register_values, u32 max, u32 offset_pci_dev, u32 offset_io_base)
{
	u32 i;

	for (i = 0; i < max; i += 3) {
		pci_devfn_t dev;
		u32 where;
		unsigned long reg;
		dev = (register_values[i] & ~0xfff) + offset_pci_dev;
		where = register_values[i] & 0xfff;
		reg = pci_read_config32(dev, where);
		reg &= register_values[i+1];
		reg |= register_values[i+2] + offset_io_base;
		pci_write_config32(dev, where, reg);
	}
}

void setup_resource_map_x_offset(const u32 *register_values, u32 max, u32 offset_pci_dev, u32 offset_io_base)
{
	u32 i;

	if (RES_DEBUG)
		printk(BIOS_DEBUG, "setting up resource map ex offset....\n");

	for (i = 0; i < max; i += 4) {
		if (RES_DEBUG)
			printk(BIOS_DEBUG, "%04x: %02x %08x <- & %08x | %08x\n",
				i/4, register_values[i],
				register_values[i+1] + ((register_values[i]==RES_PCI_IO) ? offset_pci_dev : 0),
				register_values[i+2],
				register_values[i+3] + (((register_values[i] & RES_PORT_IO_32) == RES_PORT_IO_32) ? offset_io_base : 0)
				);
		switch (register_values[i]) {
		case RES_PCI_IO: //PCI
			{
			pci_devfn_t dev;
			u32 where;
			u32 reg;
			dev = (register_values[i+1] & ~0xfff) + offset_pci_dev;
			where = register_values[i+1] & 0xfff;
			reg = pci_read_config32(dev, where);
			if (RES_DEBUG)
				printk(BIOS_SPEW, "WAS: %08x\n", reg);
			reg &= register_values[i+2];
			reg |= register_values[i+3];
			pci_write_config32(dev, where, reg);
			if (RES_DEBUG)
				printk(BIOS_SPEW, "NOW: %08x\n", reg);
			}
			break;
		case RES_PORT_IO_8: // io 8
			{
			u32 where;
			u32 reg;
			where = register_values[i+1] + offset_io_base;
			reg = inb(where);
			if (RES_DEBUG)
				printk(BIOS_SPEW, "WAS: %08x\n", reg);
			reg &= register_values[i+2];
			reg |= register_values[i+3];
			outb(reg, where);
			if (RES_DEBUG)
				printk(BIOS_SPEW, "NOW: %08x\n", reg);
			}
			break;
		case RES_PORT_IO_32:  //io32
			{
			u32 where;
			u32 reg;
			where = register_values[i+1] + offset_io_base;
			reg = inl(where);
			if (RES_DEBUG)
				printk(BIOS_SPEW, "WAS: %08x\n", reg);
			reg &= register_values[i+2];
			reg |= register_values[i+3];
			outl(reg, where);
			if (RES_DEBUG)
				printk(BIOS_SPEW, "NOW: %08x\n", reg);
			}
			break;
		}
	}

	if (RES_DEBUG)
		printk(BIOS_DEBUG, "done.\n");
}

void setup_resource_map_x(const u32 *register_values, u32 max)
{
	u32 i;

	if (RES_DEBUG)
		printk(BIOS_DEBUG, "setting up resource map ex offset....\n");

	for (i = 0; i < max; i += 4) {
		if (RES_DEBUG)
			printk(BIOS_DEBUG, "%04x: %02x %08x <- & %08x | %08x\n",
				i/4, register_values[i],register_values[i+1], register_values[i+2], register_values[i+3]);
		switch (register_values[i]) {
		case RES_PCI_IO: //PCI
			{
			pci_devfn_t dev;
			u32 where;
			u32 reg;
			dev = register_values[i+1] & ~0xfff;
			where = register_values[i+1] & 0xfff;
			reg = pci_read_config32(dev, where);
			reg &= register_values[i+2];
			reg |= register_values[i+3];
			pci_write_config32(dev, where, reg);
			}
			break;
		case RES_PORT_IO_8: // io 8
			{
			u32 where;
			u32 reg;
			where = register_values[i+1];
			reg = inb(where);
			reg &= register_values[i+2];
			reg |= register_values[i+3];
			outb(reg, where);
			}
			break;
		case RES_PORT_IO_32:  //io32
			{
			u32 where;
			u32 reg;
			where = register_values[i+1];
			reg = inl(where);
			reg &= register_values[i+2];
			reg |= register_values[i+3];
			outl(reg, where);
			}
			break;
		}
	}

	if (RES_DEBUG)
		printk(BIOS_DEBUG, "done.\n");
}

/* Default resource map */
__weak void setup_mb_resource_map(void)
{
	static const u32 register_values[] = {
		/* Careful set limit registers before base registers which contain
		 the enables */
		/* DRAM Limit i Registers
		 * F1:0x44 i = 0
		 * F1:0x4C i = 1
		 * F1:0x54 i = 2
		 * F1:0x5C i = 3
		 * F1:0x64 i = 4
		 * F1:0x6C i = 5
		 * F1:0x74 i = 6
		 * F1:0x7C i = 7
		 * [ 2: 0] Destination Node ID
		 *	   000 = Node 0
		 *	   001 = Node 1
		 *	   010 = Node 2
		 *	   011 = Node 3
		 *	   100 = Node 4
		 *	   101 = Node 5
		 *	   110 = Node 6
		 *	   111 = Node 7
		 * [ 7: 3] Reserved
		 * [10: 8] Interleave select
		 *	   specifies the values of A[14:12] to use with
		 *	   interleave enable.
		 * [15:11] Reserved
		 * [31:16] DRAM Limit Address i Bits 39-24
		 *	   This field defines the upper address bits of a 40 bit
		 *	   address that define the end of the DRAM region.
		 */
		ADDRMAP_REG(0x44), 0x0000f8f8, 0x00000000,
		ADDRMAP_REG(0x4C), 0x0000f8f8, 0x00000001,
		ADDRMAP_REG(0x54), 0x0000f8f8, 0x00000002,
		ADDRMAP_REG(0x5C), 0x0000f8f8, 0x00000003,
		ADDRMAP_REG(0x64), 0x0000f8f8, 0x00000004,
		ADDRMAP_REG(0x6C), 0x0000f8f8, 0x00000005,
		ADDRMAP_REG(0x74), 0x0000f8f8, 0x00000006,
		ADDRMAP_REG(0x7C), 0x0000f8f8, 0x00000007,
		/* DRAM Base i Registers
		 * F1:0x40 i = 0
		 * F1:0x48 i = 1
		 * F1:0x50 i = 2
		 * F1:0x58 i = 3
		 * F1:0x60 i = 4
		 * F1:0x68 i = 5
		 * F1:0x70 i = 6
		 * F1:0x78 i = 7
		 * [ 0: 0] Read Enable
		 *	   0 = Reads Disabled
		 *	   1 = Reads Enabled
		 * [ 1: 1] Write Enable
		 *	   0 = Writes Disabled
		 *	   1 = Writes Enabled
		 * [ 7: 2] Reserved
		 * [10: 8] Interleave Enable
		 *	   000 = No interleave
		 *	   001 = Interleave on A[12] (2 nodes)
		 *	   010 = reserved
		 *	   011 = Interleave on A[12] and A[14] (4 nodes)
		 *	   100 = reserved
		 *	   101 = reserved
		 *	   110 = reserved
		 *	   111 = Interleve on A[12] and A[13] and A[14] (8 nodes)
		 * [15:11] Reserved
		 * [13:16] DRAM Base Address i Bits 39-24
		 *	   This field defines the upper address bits of a 40-bit
		 *	   address that define the start of the DRAM region.
		 */
		ADDRMAP_REG(0x40), 0x0000f8fc, 0x00000000,
		ADDRMAP_REG(0x48), 0x0000f8fc, 0x00000000,
		ADDRMAP_REG(0x50), 0x0000f8fc, 0x00000000,
		ADDRMAP_REG(0x58), 0x0000f8fc, 0x00000000,
		ADDRMAP_REG(0x60), 0x0000f8fc, 0x00000000,
		ADDRMAP_REG(0x68), 0x0000f8fc, 0x00000000,
		ADDRMAP_REG(0x70), 0x0000f8fc, 0x00000000,
		ADDRMAP_REG(0x78), 0x0000f8fc, 0x00000000,

		/* Memory-Mapped I/O Limit i Registers
		 * F1:0x84 i = 0
		 * F1:0x8C i = 1
		 * F1:0x94 i = 2
		 * F1:0x9C i = 3
		 * F1:0xA4 i = 4
		 * F1:0xAC i = 5
		 * F1:0xB4 i = 6
		 * F1:0xBC i = 7
		 * [ 2: 0] Destination Node ID
		 *	   000 = Node 0
		 *	   001 = Node 1
		 *	   010 = Node 2
		 *	   011 = Node 3
		 *	   100 = Node 4
		 *	   101 = Node 5
		 *	   110 = Node 6
		 *	   111 = Node 7
		 * [ 3: 3] Reserved
		 * [ 5: 4] Destination Link ID
		 *	   00 = Link 0
		 *	   01 = Link 1
		 *	   10 = Link 2
		 *	   11 = Reserved
		 * [ 6: 6] Reserved
		 * [ 7: 7] Non-Posted
		 *	   0 = CPU writes may be posted
		 *	   1 = CPU writes must be non-posted
		 * [31: 8] Memory-Mapped I/O Limit Address i (39-16)
		 *	   This field defines the upp address bits of a 40-bit
		 *	   address that defines the end of a memory-mapped
		 *	   I/O region n
		 */
		ADDRMAP_REG(0x84), 0x00000048, 0x00000000,
		ADDRMAP_REG(0x8C), 0x00000048, 0x00000000,
		ADDRMAP_REG(0x94), 0x00000048, 0x00000000,
		ADDRMAP_REG(0x9C), 0x00000048, 0x00000000,
		ADDRMAP_REG(0xA4), 0x00000048, 0x00000000,
		ADDRMAP_REG(0xAC), 0x00000048, 0x00000000,
		ADDRMAP_REG(0xB4), 0x00000048, 0x00000000,
		ADDRMAP_REG(0xBC), 0x00000048, 0x00ffff00,

		/* Memory-Mapped I/O Base i Registers
		 * F1:0x80 i = 0
		 * F1:0x88 i = 1
		 * F1:0x90 i = 2
		 * F1:0x98 i = 3
		 * F1:0xA0 i = 4
		 * F1:0xA8 i = 5
		 * F1:0xB0 i = 6
		 * F1:0xB8 i = 7
		 * [ 0: 0] Read Enable
		 *	   0 = Reads disabled
		 *	   1 = Reads Enabled
		 * [ 1: 1] Write Enable
		 *	   0 = Writes disabled
		 *	   1 = Writes Enabled
		 * [ 2: 2] Cpu Disable
		 *	   0 = Cpu can use this I/O range
		 *	   1 = Cpu requests do not use this I/O range
		 * [ 3: 3] Lock
		 *	   0 = base/limit registers i are read/write
		 *	   1 = base/limit registers i are read-only
		 * [ 7: 4] Reserved
		 * [31: 8] Memory-Mapped I/O Base Address i (39-16)
		 *	   This field defines the upper address bits of a 40bit
		 *	   address that defines the start of memory-mapped
		 *	   I/O region i
		 */
		ADDRMAP_REG(0x80), 0x000000f0, 0x00000000,
		ADDRMAP_REG(0x88), 0x000000f0, 0x00000000,
		ADDRMAP_REG(0x90), 0x000000f0, 0x00000000,
		ADDRMAP_REG(0x98), 0x000000f0, 0x00000000,
		ADDRMAP_REG(0xA0), 0x000000f0, 0x00000000,
		ADDRMAP_REG(0xA8), 0x000000f0, 0x00000000,
		ADDRMAP_REG(0xB0), 0x000000f0, 0x00000000,
		ADDRMAP_REG(0xB8), 0x000000f0, 0x00fc0003,

		/* PCI I/O Limit i Registers
		 * F1:0xC4 i = 0
		 * F1:0xCC i = 1
		 * F1:0xD4 i = 2
		 * F1:0xDC i = 3
		 * [ 2: 0] Destination Node ID
		 *	   000 = Node 0
		 *	   001 = Node 1
		 *	   010 = Node 2
		 *	   011 = Node 3
		 *	   100 = Node 4
		 *	   101 = Node 5
		 *	   110 = Node 6
		 *	   111 = Node 7
		 * [ 3: 3] Reserved
		 * [ 5: 4] Destination Link ID
		 *	   00 = Link 0
		 *	   01 = Link 1
		 *	   10 = Link 2
		 *	   11 = reserved
		 * [11: 6] Reserved
		 * [24:12] PCI I/O Limit Address i
		 *	   This field defines the end of PCI I/O region n
		 * [31:25] Reserved
		 */
		ADDRMAP_REG(0xC4), 0xFE000FC8, 0x01fff000,
		ADDRMAP_REG(0xCC), 0xFE000FC8, 0x00000000,
		ADDRMAP_REG(0xD4), 0xFE000FC8, 0x00000000,
		ADDRMAP_REG(0xDC), 0xFE000FC8, 0x00000000,

		/* PCI I/O Base i Registers
		 * F1:0xC0 i = 0
		 * F1:0xC8 i = 1
		 * F1:0xD0 i = 2
		 * F1:0xD8 i = 3
		 * [ 0: 0] Read Enable
		 *	   0 = Reads Disabled
		 *	   1 = Reads Enabled
		 * [ 1: 1] Write Enable
		 *	   0 = Writes Disabled
		 *	   1 = Writes Enabled
		 * [ 3: 2] Reserved
		 * [ 4: 4] VGA Enable
		 *	   0 = VGA matches Disabled
		 *	   1 = matches all address < 64K and where A[9:0] is in
		 *	       the range 3B0-3BB or 3C0-3DF independent of the
		 *	        base & limit registers
		 * [ 5: 5] ISA Enable
		 *	   0 = ISA matches Disabled
		 *	   1 = Blocks address < 64K and in the last 768 bytes of
		 *	       eack 1K block from matching agains this base/limit
		 *	       pair
		 * [11: 6] Reserved
		 * [24:12] PCI I/O Base i
		 *	   This field defines the start of PCI I/O region n
		 * [31:25] Reserved
		 */
		ADDRMAP_REG(0xC0), 0xFE000FCC, 0x00000003,
		ADDRMAP_REG(0xC8), 0xFE000FCC, 0x00000000,
		ADDRMAP_REG(0xD0), 0xFE000FCC, 0x00000000,
		ADDRMAP_REG(0xD8), 0xFE000FCC, 0x00000000,

		/* Config Base and Limit i Registers
		 * F1:0xE0 i = 0
		 * F1:0xE4 i = 1
		 * F1:0xE8 i = 2
		 * F1:0xEC i = 3
		 * [ 0: 0] Read Enable
		 *	   0 = Reads Disabled
		 *	   1 = Reads Enabled
		 * [ 1: 1] Write Enable
		 *	   0 = Writes Disabled
		 *	   1 = Writes Enabled
		 * [ 2: 2] Device Number Compare Enable
		 *	   0 = The ranges are based on bus number
		 *	   1 = The ranges are ranges of devices on bus 0
		 * [ 3: 3] Reserved
		 * [ 6: 4] Destination Node
		 *	   000 = Node 0
		 *	   001 = Node 1
		 *	   010 = Node 2
		 *	   011 = Node 3
		 *	   100 = Node 4
		 *	   101 = Node 5
		 *	   110 = Node 6
		 *	   111 = Node 7
		 * [ 7: 7] Reserved
		 * [ 9: 8] Destination Link
		 *	   00 = Link 0
		 *	   01 = Link 1
		 *	   10 = Link 2
		 *	   11 - Reserved
		 * [15:10] Reserved
		 * [23:16] Bus Number Base i
		 *	   This field defines the lowest bus number in
		 *	    configuration region i
		 * [31:24] Bus Number Limit i
		 *	   This field defines the highest bus number in
		 *	   configuration regin i
		 */
		ADDRMAP_REG(0xE0), 0x0000FC88, 0xff000003,
		ADDRMAP_REG(0xE4), 0x0000FC88, 0x00000000,
		ADDRMAP_REG(0xE8), 0x0000FC88, 0x00000000,
		ADDRMAP_REG(0xEC), 0x0000FC88, 0x00000000,
	};

	u32 max;
	max = ARRAY_SIZE(register_values);
	setup_resource_map(register_values, max);
}

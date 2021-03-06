/* SPDX-License-Identifier: GPL-2.0-only */

#include <acpi/acpi.h>
#include <arch/ioapic.h>
#include <device/pci_ops.h>

unsigned long acpi_fill_madt(unsigned long current)
{
	struct device *dev;
	u32 dword;
	u32 gsi_base = 0;
	uint32_t apicid_sp5100;
	uint32_t apicid_sr5650;
	/* create all subtables for processors */
	current = acpi_create_madt_lapics(current);

	if (CONFIG(ENABLE_APIC_EXT_ID) && (CONFIG_APIC_ID_OFFSET > 0))
		apicid_sp5100 = 0x0;
	else
		apicid_sp5100 = 0x20;
	apicid_sr5650 = apicid_sp5100 + 1;

	/* Write SB700 IOAPIC, only one */
	current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *) current, apicid_sp5100,
					   IO_APIC_ADDR, gsi_base);
	/* IOAPIC on sr56x0 */
	gsi_base += 24;		/* SB700 has 24 IOAPIC entries. */
	dev = pcidev_on_root(0, 0);
	if (dev) {
		pci_write_config32(dev, 0xF8, 0x1);
		dword = pci_read_config32(dev, 0xFC) & 0xfffffff0;
		current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *) current,
						   apicid_sr5650, dword, gsi_base);
	}

	/* bus, source, gsirq, flags */
	current += acpi_create_madt_irqoverride((acpi_madt_irqoverride_t *)
						current, 0, 0, 2, 0);
	current += acpi_create_madt_irqoverride((acpi_madt_irqoverride_t *)
						current, 0, 9, 9, 0xf);

	/* create all subtables for processors */
	current += acpi_create_madt_lapic_nmi((acpi_madt_lapic_nmi_t *)current, 0xff, 0, 1);
	/* 1: LINT1 connect to NMI */

	return current;
}

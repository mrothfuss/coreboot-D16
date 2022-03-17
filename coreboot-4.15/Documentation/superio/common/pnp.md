# PNP devices

Typical PNP devices are Super I/Os, LPC-connected TPMs and board
management controllers.

PNP devices are usually connected to the LPC or eSPI bus of a system
and shouldn't be confused with PCI(e) devices that use a completely
different plug and play mechanism. PNP originates in the ISA plug and
play specification. Since the original ISA bus is more or less extinct,
the auto-detection part of ISA PNP is mostly irrelevant nowadays. For
the register offsets for different functionality, appendix A of that
specification is still the main reference though.

## Configuration access and config mode

Super I/O chips connected via LPC to the southbridge usually have their
I/O-mapped configuration interface with a size of two bytes at the base
address `0x2e` or `0x4e`. Other PNP devices have their configuration
interface at other addresses.

The two byte registers allow access to an indirect 256 bytes big
register space that contains the configuration. By writing the index to
the lower byte (e.g. `0x2e`), you can access the register contents at
that index by reading/writing the higher byte (e.g. `0x2f`).

To prevent accidental changes of the Super I/O (SIO) configuration,
the SIOs need a configuration mode unlock sequence. After changing the
configuration, the configuration mode should be left again, by sending
the configuration mode lock sequence.

## Logical device numbers (LDN)

Each PNP device can contain multiple logical devices. The bytes from
`0x00` to `0x2f` in the indirect configuration register space are common
for all LDNs, but some SIO chips require a certain LDN to be selected in
order to write certain registers in there. An LDN gets selected by
writing the LDN number to the LDN select register `0x07`. Registers
`0x30` to `0xff` are specific to each LDN number.

coreboot encodes the physical LDN number in the lower byte of the LDN
number.

### Virtual logical device numbers

Register `0x30` is the LDN enable register and since it is an 8 bit
register, it can contain up to 8 enable bits for different parts of
the functionality of that logical device. To set a certain enable bit
in one physical LDN, the concept of virtual LDNs was introduced.
Virtual LDNs share the registers of their base LDN, but allow to
specify which part of a LDN should be enabled.

coreboot encodes the enable bit number and by that the virtual LDN
part in the lower 3 bits of the higher byte of the LDN number.

## I/O resources

Starting at register address `0x60`, each LDN has 2 byte wide I/O base
address registers. The size of an I/O resource is always a power of
two.

### I/O resource masks

The I/O resource masks encode both the size and the maximum base
address of the corresponding IO resource. The number of zeros counted
from the least significant bit encode the resource size. If N is the
number of LSBs being zero, which can also be zero if the LSB is a one,
the resource has N address bits and a size of 2\*\*N bytes. The mask
address is also the highest possible address to map the I/O region.

A typical example for an I/O resource mask is `0x07f8` which is
`0b0000011111111000` in binary notation. The three LSBs are zeros here,
so it's an eight byte I/O resource with three address offset bits
inside the resource. The highest base address it can be mapped to is
`0x07f8`, so the region will end at `0x07ff`.

The Super I/O datasheets typically contain the information about the
I/O resource masks. On most Super I/O chips the mask can also be found
out by writing `0xffff` to the corresponding I/O base address register
and reading back the value; since the lowest and highest bits are
hard-wired to zero according to the I/O resource size and maximal
possible I/O address, this gives the mask.

## IRQ resources

Each physical LDN has up to two configurable interrupt request register
pairs `0x70`, `0x71` and `0x72`, `0x73`. Each pair can be configured to
use a certain IRQ number. Writing 1 to 15 into the first register
selects the IRQ number generated by the corresponding IRQ source and
enables IRQ generation; writing 0 to it disables the generation of IRQs
for the source. The second register selects the IRQ type (level or edge)
and IRQ level (high or low). For LPC SIOs the IRQ type is hard-wired to
edge.

On the LPC bus a shared SERIRQ line is used to signal IRQs to the
host; the IRQ number gets encoded by the number of LPC clock cycles
after the start frame before the device pulls the open drain
connection low.

SERIRQ can be used in two different modes: In the continuous SERIRQ
mode the host continuously sends IRQ frame starts and the devices
signal their IRQ request by pulling low the SERIRQ line at the right
time. In quiet SERIRQ mode the host doesn't send IRQ frame starts, so
the devices have to send both the IRQ frame start and the encoded IRQ
number. The quiet mode is often broken.

## DRQ resources

Each physical LDN has two legacy ISA-style DMA request channel
registers at `0x74` and `0x75`. Those are only used for legacy devices
like parallel printer ports or floppy disk controllers.

Each device using LPC legacy DMA needs its own LDMA line to the host.
Some newer chipsets have dropped the LDMA line and with that the
legacy DMA capability on LPC.
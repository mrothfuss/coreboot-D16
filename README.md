# coreboot-D16

coreboot is a Free Software project aimed at replacing the proprietary BIOS
(firmware) found in most computers.  coreboot performs a little bit of
hardware initialization and then executes additional boot logic, called a
payload.

This repository is dedicated to maintaining/building coreboot for the
ASUS KGPE-D16 motherboard, which has lost upstream support in coreboot
... and was never very stable to begin with (ie: boot stalls, runtime crashes, memory issues).

Information is also provided to assist others in building reliable firmware
for the D16 motherboard. The included files are configured to build a firmware
image without any closed-source components. Example commands are provided to customize the firmware
and include various closed-source components.

## coreboot-4.11

### Status

Missing Features: no fan control

Consistency: the boot process is reliable (1000+ boots with no hangs/issues)

Stability: about 1/50 boots will be unstable and crash under very high load


### Build Environment

Debian 9 or 10 (amd64)

```bash
apt-get install -y bison build-essential curl flex git gnat libncurses5-dev m4 zlib1g-dev acpica-tools libgmp-dev libmpfr-dev libmpc-dev
```

### Codebase

* [coreboot-4.11 source code](https://www.coreboot.org/downloads.html)
* The mainboard specific code was originally developed by [Raptor Engineering](https://www.raptorengineering.com/coreboot/kgpe-d16-status.php)
* A few patches developed here to fix bugs and improve stability

## Building coreboot

```bash
### switch to the desired version
# cd coreboot-D16/coreboot-4.11

### build the cross compiler
make crossgcc-i386 CPUS=$(nproc)

### configure the firmware (optional)
# make menuconfig

### build the coreboot rom
make
```

## Fetching blobs

```bash
# cd coreboot-D16

### CPU Microcode
mkdir -p blobs/amd && cd blobs/amd
curl -O "https://httpredir.debian.org/debian/pool/non-free/a/amd64-microcode/amd64-microcode_3.20191218.1.tar.xz"
tar xf amd64-microcode_3.20191218.1.tar.xz
cd ../../

### ASUS VGABIOS
mkdir -p blobs/asus && cd blobs/asus
curl -O "https://dlcdnets.asus.com/pub/ASUS/mb/SocketG34(1944)/KGPE-D16/BIOS/KGPE-D16-ASUS-3309.zip"
unzip KGPE-D16-ASUS-3309.zip
dd if=./KGPE-D16-ASUS-3309.ROM of=./vgabios.bin bs=1024 skip=1856 count=32
cd ../../
```

## Configuring the ROM

```bash
### switch to the desired version
# cd coreboot-D16/coreboot-4.11

# copy the base firmware image for modification
cp ./build/coreboot.rom ./

# set SeaBIOS option ROM settings
# 0 = no roms, 1 = VGA only, 2 = all roms (default)
./build/cbfstool coreboot.rom add-int -i 1 -n etc/pci-optionrom-exec

# set SeaBIOS boot menu wait time (ms)
./build/cbfstool coreboot.rom add-int -i 10000 -n etc/boot-menu-wait

# add AMD microcode
./build/cbfstool coreboot.rom add -n microcode_amd.bin -f ../blobs/amd/amd64-microcode-3.20191218.1/microcode_amd.bin -t microcode
./build/cbfstool coreboot.rom add -n microcode_amd_fam15h.bin -f ../blobs/amd/amd64-microcode-3.20191218.1/microcode_amd_fam15h.bin -t microcode

# replace SeaVGABIOS with ASUS VGABIOS
./build/cbfstool coreboot.rom remove -n vgaroms/seavgabios.bin
./build/cbfstool coreboot.rom add -n pci1a03,2000.rom -f ../blobs/asus/vgabios.bin -t optionrom

# add coreboot bootsplash (requires ASUS VGABIOS)
./build/cbfstool coreboot.rom add -n bootsplash.jpg -f ../blobs/coreboot/bootsplash_fs.jpg -t bootsplash

```

## Tested Hardware

Mainboard Revisions: 1.03G, 1.04, 1.05

Flash Chips: W25Q128FVIQ

Processors: 6282, 6328, 6380, 6386

Memory: Crucial CT16G3ERSLD4160B, Samsung M393B2G70BH0-CK0

KVM Modules: ASMB4 (running openBMC)

RAID Cards: PIKE 2008 (IT Mode)

GPUs: RX 550 (PCI passthrough compatible)

PCIe: MHQJRH Dual M.2 Adapter (NVMe drive is bootable by SeaBIOS)


## Tested BLOBs

Microcode: amd64-microcode-3.20160316.3, amd64-microcode-3.20191218.1

VGA BIOS: extracted from ASUS Firmware v3309


## Resources

ASUS Motherboard Manual (https://dlcdnets.asus.com/pub/ASUS/mb/SocketG34(1944)/KGPE-D16/Menual_QVL/E8847_KGPE-D16.pdf)

AMD Fam15h BIOS and Kernel Developers Guide (https://www.amd.com/system/files/TechDocs/42301_15h_Mod_00h-0Fh_BKDG.pdf)

AMD SR5690 BIOS Developers Guide (https://www.amd.com/system/files/TechDocs/43870.pdf)

AMD SR5690 Register Programming Requirements (https://www.amd.com/system/files/TechDocs/43872.pdf)

ASUS Firmware v3309 (https://dlcdnets.asus.com/pub/ASUS/mb/SocketG34(1944)/KGPE-D16/BIOS/KGPE-D16-ASUS-3309.zip)

AMD64 Microcode (https://httpredir.debian.org/debian/pool/non-free/a/amd64-microcode/)

Raptor Engineering OpenBMC (https://www.raptorengineering.com/coreboot/kgpe-d16-bmc-port-status.php)

Coreboot VBIOS Extraction (https://www.coreboot.org/VGA_support#How_to_retrieve_a_good_video_bios)

ASUS PIKE 2008 IT Mode (https://gist.github.com/pjobson/9ec25f7fc991f28d132ca813ab1bd541)


## Copyright and License

The copyright on coreboot is owned by quite a large number of individual
developers and companies. Please check the individual source files for details.

coreboot is licensed under the terms of the GNU General Public License (GPL).
Some files are licensed under the "GPL (version 2, or any later version)",
and some files are licensed under the "GPL, version 2". For some parts, which
were derived from other projects, other (GPL-compatible) licenses may apply.
Please check the individual source files for details.

This makes the resulting coreboot images licensed under the GPL, version 2.

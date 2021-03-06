/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef SB700_SMBUS_H
#define SB700_SMBUS_H

#include <stdint.h>
#include "stddef.h"

#define SMBUS_IO_BASE 0xb00
#define SMBUS_AUX_IO_BASE 0xb20

#define SMBHSTSTAT 0x0
#define SMBSLVSTAT 0x1
#define SMBHSTCTRL 0x2
#define SMBHSTCMD  0x3
#define SMBHSTADDR 0x4
#define SMBHSTDAT0 0x5
#define SMBHSTDAT1 0x6
#define SMBHSTBLKDAT 0x7

#define SMBSLVCTRL 0x8
#define SMBSLVCMD_SHADOW 0x9
#define SMBSLVEVT 0xa
#define SMBSLVDAT 0xc
#define SMBSLVMISC 0xd

#define AX_INDXC  0
#define AX_INDXP  1
#define AXCFG     2
#define ABCFG     3

#define AB_INDX   0xCD8
#define AB_DATA   (AB_INDX+4)

/* Between 1-10 seconds, We should never timeout normally
 * Longer than this is just painful when a timeout condition occurs.
 */
#define SMBUS_TIMEOUT (100*1000*10)

#define abcfg_reg(reg, mask, val)	\
	alink_ab_indx((ABCFG), (reg), (mask), (val))
#define axcfg_reg(reg, mask, val)	\
	alink_ab_indx((AXCFG), (reg), (mask), (val))
#define axindxc_reg(reg, mask, val)	\
	alink_ax_indx(0, (reg), (mask), (val))
#define axindxp_reg(reg, mask, val)	\
	alink_ax_indx(1, (reg), (mask), (val))

void alink_ab_indx(u32 reg_space, u32 reg_addr, u32 mask, u32 val);
void alink_ax_indx(u32 space, u32 axindc, u32 mask, u32 val);
int do_smbus_recv_byte(u32 smbus_io_base, u32 device);
int do_smbus_send_byte(u32 smbus_io_base, u32 device, u8 val);
int do_smbus_read_byte(u32 smbus_io_base, u32 device, u32 address);
int do_smbus_write_byte(u32 smbus_io_base, u32 device, u32 address, u8 val);

#endif

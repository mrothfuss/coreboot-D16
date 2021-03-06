/* SPDX-License-Identifier: GPL-2.0-only */

/* Low swap bit vs bank size encoding (physical, not logical address bit)
 * ;To calculate the number by hand, add the number of Bank address bits
 * ;(2 or 3) to the number of column address bits, plus 3 (the logical
 * ;page size), and subtract 8.
 */

#include <stdint.h>
#include <console/console.h>
#include <drivers/amd/amdmct/wrappers/mcti.h>
#include "mct_d_gcc.h"


static const u8 Tab_int_D[] = {6,7,7,8,8,8,8,8,9,9,8,9};

void InterleaveBanks_D(struct MCTStatStruc *pMCTstat,
			struct DCTStatStruc *pDCTstat, u8 dct)
{
	u8 ChipSel, EnChipSels;
	u32 AddrLoMask, AddrHiMask;
	u32 AddrLoMaskN, AddrHiMaskN, MemSize = 0;
	u8 DoIntlv, _CsIntCap;
	u32 BitDelta, BankEncd = 0;

	u32 dev;
	u32 reg;
	u32 val;
	u32 val_lo, val_hi;

	DoIntlv = mctGet_NVbits(NV_BankIntlv);
	_CsIntCap = 0;
	EnChipSels = 0;

	dev = pDCTstat->dev_dct;

	ChipSel = 0;		/* Find out if current configuration is capable */
	while (DoIntlv && (ChipSel < MAX_CS_SUPPORTED)) {
		reg = 0x40+(ChipSel<<2);	/* Dram CS Base 0 */
		val = Get_NB32_DCT(dev, dct, reg);
		if (val & (1<<CSEnable)) {
			EnChipSels++;
			reg = 0x60+((ChipSel>>1)<<2); /*Dram CS Mask 0 */
			val = Get_NB32_DCT(dev, dct, reg);
			val >>= 19;
			val &= 0x3ff;
			val++;
			if (EnChipSels == 1)
				MemSize = val;
			else
				/*If mask sizes not same then skip */
				if (val != MemSize)
					break;
			reg = 0x80;		/*Dram Bank Addressing */
			val = Get_NB32_DCT(dev, dct, reg);
			val >>= (ChipSel>>1)<<2;
			val &= 0x0f;
			if (EnChipSels == 1)
				BankEncd = val;
			else
				/*If number of Rows/Columns not equal, skip */
				if (val != BankEncd)
					break;
		}
		ChipSel++;
	}
	if (ChipSel == MAX_CS_SUPPORTED) {
		if ((EnChipSels == 2) || (EnChipSels == 4) || (EnChipSels == 8))
			_CsIntCap = 1;
	}

	if (DoIntlv) {
		if (!_CsIntCap) {
			pDCTstat->ErrStatus |= 1<<SB_BkIntDis;
			DoIntlv = 0;
		}
	}

	if (DoIntlv) {
		val = Tab_int_D[BankEncd];
		if (pDCTstat->Status & (1<<SB_128bitmode))
			val++;

		AddrLoMask = (EnChipSels - 1)  << val;
		AddrLoMaskN = ~AddrLoMask;

		val = bsf(MemSize) + 19;
		AddrHiMask = (EnChipSels -1) << val;
		AddrHiMaskN = ~AddrHiMask;

		BitDelta = bsf(AddrHiMask) - bsf(AddrLoMask);

		for (ChipSel = 0; ChipSel < MAX_CS_SUPPORTED; ChipSel++) {
			reg = 0x40 + (ChipSel<<2);	/* Dram CS Base 0 */
			val = Get_NB32_DCT(dev, dct, reg);
			if (val & 3) {
				val_lo = val & AddrLoMask;
				val_hi = val & AddrHiMask;
				val &= AddrLoMaskN;
				val &= AddrHiMaskN;
				val_lo <<= BitDelta;
				val_hi >>= BitDelta;
				val |= val_lo;
				val |= val_hi;
				Set_NB32_DCT(dev, dct, reg, val);

				if (ChipSel & 1)
					continue;

				reg = 0x60 + ((ChipSel>>1)<<2); /* Dram CS Mask 0 */
				val = Get_NB32_DCT(dev, dct, reg);
				val_lo = val & AddrLoMask;
				val_hi = val & AddrHiMask;
				val &= AddrLoMaskN;
				val &= AddrHiMaskN;
				val_lo <<= BitDelta;
				val_hi >>= BitDelta;
				val |= val_lo;
				val |= val_hi;
				Set_NB32_DCT(dev, dct, reg, val);
			}
		}
	}	/* DoIntlv */

	/* dump_pci_device(PCI_DEV(0, 0x18+pDCTstat->Node_ID, 2)); */

	printk(BIOS_DEBUG, "InterleaveBanks_D: Status %x\n", pDCTstat->Status);
	printk(BIOS_DEBUG, "InterleaveBanks_D: ErrStatus %x\n", pDCTstat->ErrStatus);
	printk(BIOS_DEBUG, "InterleaveBanks_D: ErrCode %x\n", pDCTstat->ErrCode);
	printk(BIOS_DEBUG, "InterleaveBanks_D: Done\n\n");
}

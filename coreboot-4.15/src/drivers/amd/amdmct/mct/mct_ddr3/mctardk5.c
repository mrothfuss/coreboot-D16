/* SPDX-License-Identifier: GPL-2.0-only */

/* AM3/ASB2/C32/G34 DDR3 */

#include <arch/cpu.h>
#include <stdint.h>
#include <drivers/amd/amdmct/wrappers/mcti.h>
#include "mct_d_gcc.h"

static void Get_ChannelPS_Cfg0_D(u8 MAAdimms, u8 Speed, u8 MAAload,
				u32 *ODC_CTL,
				u8 *CMDmode);

void mctGet_PS_Cfg_D(struct MCTStatStruc *pMCTstat,
			 struct DCTStatStruc *pDCTstat, u32 dct)
{
	if (is_fam15h()) {
		pDCTstat->CH_ADDR_TMG[dct] = fam15h_address_timing_compensation_code(pDCTstat, dct);
		pDCTstat->CH_ODC_CTL[dct] = fam15h_output_driver_compensation_code(pDCTstat, dct);
		pDCTstat->_2Tmode = fam15h_slow_access_mode(pDCTstat, dct);
	} else {
		Get_ChannelPS_Cfg0_D(pDCTstat->MAdimms[dct], pDCTstat->Speed,
					pDCTstat->MAload[dct],
					&(pDCTstat->CH_ODC_CTL[dct]),
					&pDCTstat->_2Tmode);

		if (pDCTstat->Status & (1 << SB_Registered)) {
			pDCTstat->_2Tmode = 1;	/* Disable slow access mode */
		}
		pDCTstat->CH_ADDR_TMG[dct] = fam10h_address_timing_compensation_code(pDCTstat, dct);

		pDCTstat->CH_ODC_CTL[dct] |= 0x20000000;	/* 60ohms */
	}

	pDCTstat->CH_EccDQSLike[0]  = 0x0403;
	pDCTstat->CH_EccDQSScale[0] = 0x70;
	pDCTstat->CH_EccDQSLike[1]  = 0x0403;
	pDCTstat->CH_EccDQSScale[1] = 0x70;
}

/*
 *  In: MAAdimms   - number of DIMMs on the channel
 *    : Speed      - Speed (see DCTStatstruc.Speed for definition)
 *    : MAAload    - number of address bus loads on the channel
 * Out: AddrTmgCTL - Address Timing Control Register Value
 *    : ODC_CTL    - Output Driver Compensation Control Register Value
 *    : CMDmode    - CMD mode
 */
static void Get_ChannelPS_Cfg0_D(u8 MAAdimms, u8 Speed, u8 MAAload,
				u32 *ODC_CTL,
				u8 *CMDmode)
{
	*ODC_CTL = 0;
	*CMDmode = 1;

	if (MAAdimms == 1) {
		*ODC_CTL = 0x00113222;
		*CMDmode = 1;
	} else /* if (MAAdimms == 0) */ {
		if (Speed == 4) {
			*CMDmode = 1;
		} else if (Speed == 5) {
			*CMDmode = 1;
		} else if (Speed == 6) {
			*CMDmode = 2;
		} else {
			*CMDmode = 2;
		}
		*ODC_CTL = 0x00223323;
	}
}

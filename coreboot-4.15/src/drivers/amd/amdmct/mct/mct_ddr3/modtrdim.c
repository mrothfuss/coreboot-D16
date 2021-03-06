/* SPDX-License-Identifier: GPL-2.0-only */

/* This file contains functions for odt setting on registered DDR3 dimms */

#include <stdint.h>
#include <console/console.h>

#include <drivers/amd/amdmct/wrappers/mcti.h>
#include "mct_d_gcc.h"

/**
 *
 *
 *   This function set Rtt_Nom for registered DDR3 dimms on targeted dimm.
 *
 *     @param      *pMCTData
 *     @param[in]  *pDCTData - Pointer to buffer with information about each DCT
 *     @param      dimm - targeted dimm
 *     @param      wl - current mode, either write levelization mode or normal mode
 *     @param      MemClkFreq - current frequency
 *     @param      rank
 *
 *     @return     tempW1 - Rtt_Nom
 */
u32 RttNomTargetRegDimm (sMCTStruct *pMCTData, sDCTStruct *pDCTData, u8 dimm, bool wl, u8 MemClkFreq, u8 rank)
{
	u32 tempW1;
	tempW1 = 0;
	if (wl) {
		switch (mctGet_NVbits(NV_MAX_DIMMS_PER_CH)) {
		case 2:
			/* 2 dimms per channel */
			if (pDCTData->MaxDimmsInstalled == 1) {
				if ((pDCTData->DimmRanks[dimm] == 2) && (rank == 0)) {
					tempW1 = 0x00;	/* Rtt_Nom = OFF */
				} else if (pDCTData->DimmRanks[dimm] == 4) {
					if (rank == 1) {
						tempW1 = 0x00;	/* Rtt_Nom = OFF on second and forth rank of QR dimm */
					} else {
						if (MemClkFreq == 6) {
							tempW1 = 0x04;	/* Rtt_Nom = 60 ohms */
						} else {
							tempW1 = 0x40;	/* Rtt_Nom = 120 ohms */
						}
					}
				} else {
					tempW1 = 0x04;	/* Rtt_Nom = 60 ohms */
				}
			} else if (pDCTData->MaxDimmsInstalled == 2) {
				if (((pDCTData->DimmRanks[dimm] == 2) || (pDCTData->DimmRanks[dimm] == 4)) && (rank == 1)) {
					tempW1 = 0x00;	/* Rtt_Nom = OFF */
				} else if ((pDCTData->DimmRanks[dimm] == 4) || (pDCTData->DctCSPresent & 0xF0)) {
					if (MemClkFreq == 3) {
						tempW1 = 0x40;	/* Rtt_Nom = 120 ohms */
					} else {
						tempW1 = 0x04;	/* Rtt_Nom = 60 ohms */
					}
				} else {
					if (MemClkFreq == 6) {
						tempW1 = 0x04;	/* Rtt_Nom = 60 ohms */
					} else {
						tempW1 = 0x40;	/* Rtt_Nom = 120 ohms */
					}
				}
			}
			break;
		case 3:
			/* 3 dimms per channel */
			/* QR not supported in this case on L1 package. */
			if (pDCTData->MaxDimmsInstalled == 1) {
				if ((pDCTData->DimmRanks[dimm] == 2) && (rank == 1)) {
					tempW1 = 0x00;	/* Rtt_Nom = OFF */
				} else {
					tempW1 = 0x04;	/* Rtt_Nom = 60 ohms */
				}
			} else {
				tempW1 = 0x40;	/* Rtt_Nom = 120 ohms */
			}
			break;
		default:
			die("modtrdim.c: WTF?");
		}
	} else {
		switch (mctGet_NVbits(NV_MAX_DIMMS_PER_CH)) {
		case 2:
			/* 2 dimms per channel */
			if ((pDCTData->DimmRanks[dimm] == 4) && (rank == 1)) {
				tempW1 = 0x00;	/* Rtt_Nom = OFF */
			} else if ((pDCTData->MaxDimmsInstalled == 1) || (pDCTData->DimmRanks[dimm] == 4)) {
				tempW1 = 0x04;	/* Rtt_Nom = 60 ohms */
			} else {
				if (pDCTData->DctCSPresent & 0xF0) {
					tempW1 = 0x0204;	/* Rtt_Nom = 30 ohms */
				} else {
					if (MemClkFreq < 5) {
						tempW1 = 0x44;	/* Rtt_Nom = 40 ohms */
					} else {
						tempW1 = 0x0204;	/* Rtt_Nom = 30 ohms */
					}
				}
			}
			break;
		case 3:
			/* 3 dimms per channel */
			/* L1 package does not support QR dimms this case. */
			if (rank == 1) {
				tempW1 = 0x00;	/* Rtt_Nom = OFF */
			} else if (pDCTData->MaxDimmsInstalled == 1) {
				tempW1 = 0x04;	/* Rtt_Nom = 60 ohms */
			} else if ((MemClkFreq < 5) || (pDCTData->MaxDimmsInstalled == 3)) {
				tempW1 = 0x44;	/* Rtt_Nom = 40 ohms */
			} else {
				tempW1 = 0x0204;	/* Rtt_Nom = 30 ohms */
			}
			break;
		default:
			die("modtrdim.c: WTF?");
		}
	}
	return tempW1;
}

/* -----------------------------------------------------------------------------*/
/**
 *
 *
 *   This function set Rtt_Nom for registered DDR3 dimms on non-targeted dimm.
 *
 *     @param      *pMCTData
 *     @param[in]  *pDCTData - Pointer to buffer with information about each DCT
 *     @param      dimm - non-targeted dimm
 *     @param      wl - current mode, either write levelization mode or normal mode
 *     @param      MemClkFreq - current frequency
 *     @param      rank
 *
 *      @return    tempW1 - Rtt_Nom
 */
u32 RttNomNonTargetRegDimm (sMCTStruct *pMCTData, sDCTStruct *pDCTData, u8 dimm, bool wl, u8 MemClkFreq, u8 rank)
{
	if ((wl) && (mctGet_NVbits(NV_MAX_DIMMS_PER_CH) == 2) && (pDCTData->DimmRanks[dimm] == 2) && (rank == 1)) {
		return 0x00;	/* for non-target dimm during WL, the second rank of a DR dimm need to have Rtt_Nom = OFF */
	} else {
		return RttNomTargetRegDimm (pMCTData, pDCTData, dimm, FALSE, MemClkFreq, rank);	/* otherwise, the same as target dimm in normal mode. */
	}
}

/* -----------------------------------------------------------------------------*/
/**
 *
 *
 *   This function set Rtt_Wr for registered DDR3 dimms.
 *
 *     @param      pMCTData
 *     @param[in]  *pDCTData - Pointer to buffer with information about each DCT
 *     @param      dimm - targeted dimm
 *     @param      wl - current mode, either write levelization mode or normal mode
 *     @param      MemClkFreq - current frequency
 *     @param      rank
 *
 *      @return    tempW1 - Rtt_Wr
 */

u32 RttWrRegDimm (sMCTStruct *pMCTData, sDCTStruct *pDCTData, u8 dimm, bool wl, u8 MemClkFreq, u8 rank)
{
	u32 tempW1;
	tempW1 = 0;
	if (wl) {
		tempW1 = 0x00;	/* Rtt_WR = OFF */
	} else {
		switch (mctGet_NVbits(NV_MAX_DIMMS_PER_CH)) {
		case 2:
			if (pDCTData->MaxDimmsInstalled == 1) {
				if (pDCTData->DimmRanks[dimm] != 4) {
					tempW1 = 0x00;
				} else {
					if (MemClkFreq == 6) {
						tempW1 = 0x200;	/* Rtt_WR = 60 ohms */
					} else {
						tempW1 = 0x400;	/* Rtt_WR = 120 ohms */
					}
				}
			} else {
				if ((pDCTData->DimmRanks[dimm] == 4) || (pDCTData->DctCSPresent & 0xF0)) {
					if (MemClkFreq == 3) {
						tempW1 = 0x400;	/* Rtt_WR = 120 ohms */
					} else {
						tempW1 = 0x200;	/* Rtt_WR = 60 ohms */
					}
				} else {
					if (MemClkFreq == 6) {
						tempW1 = 0x200;	/* Rtt_WR = 60 ohms */
					} else {
						tempW1 = 0x400;	/* Rtt_Nom = 120 ohms */
					}
				}
			}
			break;
		case 3:
			if (pDCTData->MaxDimmsInstalled == 1) {
				tempW1 = 0x00;	/* Rtt_WR = OFF */
			} else {
				tempW1 = 0x400;	/* Rtt_Nom = 120 ohms */
			}
			break;
		default:
			die("modtrdim.c: WTF?");
		}
	}
	return tempW1;
}

/* -----------------------------------------------------------------------------*/
/**
 *
 *
 *   This function set WrLvOdt for registered DDR3 dimms.
 *
 *     @param      *pMCTData
 *     @param[in]  *pDCTData - Pointer to buffer with information about each DCT
 *     @param      dimm - targeted dimm
 *
 *      @return    WrLvOdt
 */
u8 WrLvOdtRegDimm (sMCTStruct *pMCTData, sDCTStruct *pDCTData, u8 dimm)
{
	u8 WrLvOdt1, i;
	WrLvOdt1 = 0;
	i = 0;
	while (i < 8) {
		if (pDCTData->DctCSPresent & (1 << i)) {
			WrLvOdt1 = (u8)bitTestSet(WrLvOdt1, i/2);
		}
		i += 2;
	}
	if (mctGet_NVbits(NV_MAX_DIMMS_PER_CH) == 2) {
		if ((pDCTData->DimmRanks[dimm] == 4) && (pDCTData->MaxDimmsInstalled != 1)) {
			if (dimm >= 2) {
				WrLvOdt1 = (u8)bitTestReset (WrLvOdt1, (dimm - 2));
			} else {
				WrLvOdt1 = (u8)bitTestReset (WrLvOdt1, (dimm + 2));
			}
		} else if ((pDCTData->DimmRanks[dimm] == 2) && (pDCTData->MaxDimmsInstalled == 1)) {
			/* the case for one DR on a 2 dimms per channel is special */
			WrLvOdt1 = 0x8;
		}
	}
	return WrLvOdt1;
}

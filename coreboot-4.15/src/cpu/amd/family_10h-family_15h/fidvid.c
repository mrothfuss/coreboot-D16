/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * This file initializes the CPU cores for voltage and frequency settings
 * in the different power states.
 */
/*

checklist (functions are in this file if no source file named)
Fam10 Bios and Kernel Development Guide #31116, rev 3.48, April 22, 2010

2.4.2.6 Requirements for p-states

1.- F3x[84:80] According to table 100 : prep_fid_change

2.- COF/VID :
     2.4.2.9.1 Steps 1,3-6 and warning for 2,7 if they apply
	       fixPsNbVidBeforeWR(...)
     2.4.2.9.1 Step 8 enable_fid_change
	       We do this for all nodes, I don't understand BKDG 100% on
	       whether this is or isn't meant by "on the local
	       processor". Must be OK.
     2.4.2.9.1 Steps 9-10 (repeat 1-7 and reset) romstage.c/init_cpus ?
     2.4.2.9.1 Steps 11-12 init_fidvid_stage2
     2.4.2.9.2 DualPlane PVI : Not supported, don't know how to detect,
	       needs specific circuitry.

3.-  2.4.2.7 dualPlaneOnly(dev)

4.-  2.4.2.8 applyBoostFIDOffset(dev, nodeid)

5.-  enableNbPState1(dev)

6.- 2.4.1.7
    a) UpdateSinglePlaneNbVid()
    b) setVSRamp(), called from  prep_fid_change
    c) prep_fid_change
    d) improperly, for lack of voltage regulator details?,
	F3xA0[PsiVidEn] in defaults.h
	F3xA0[PsiVid] in init_cpus.c AMD_SetupPSIVID_d (before prep_fid_change)

7.- TODO (Core Performance Boost is only available in revision E cpus, and we
	  don't seem to support those yet, at least they don't have any
	  constant in amddefs.h )

8.- FIXME ? Transition to min Pstate according to 2.4.2.15.3 is required
    by 2.4.2.6 after warm reset. But 2.4.2.15 states that it is not required
    if the warm reset is issued by coreboot to update NbFid. So it is required
    or not ? How can I tell who issued warm reset ?
    coreboot transitions to P0 instead, which is not recommended, and does
    not follow 2.4.2.15.2 to do so.

9.- TODO Requires information on current delivery capability
    (depends on mainboard and maybe power supply ?). One might use a config
    option with the maximum number of Amperes that the board can deliver to CPU.

10.- [Multiprocessor] TODO 2.4.2.12
     [Uniprocessor] FIXME ? We call setPStateMaxVal() in init_fidvid_stage2,
     but not sure this is what is meant by "Determine the valid set of
     P-states based on enabled P-states indicated
     in MSRC001_00[68:64][PstateEn]" in 2.4.2.6-10

11.- finalPstateChange() from init_fidvid_Stage2 (BKDG says just "may", anyway)

12.- generate ACPI for p-states.
     generated in powernow_acpi.c amd_generate_powernow()

"must also be completed"

a.-  PllLockTime set in ruleset in defaults.h
     BKDG says set it "If MSRC001_00[68:64][CpuFid] is different between
     any two enabled P-states", but since it does not say "only if"
     I guess it is safe to do it always.

b.-  prep_fid_change(...)

 */

#include <console/console.h>
#include <cpu/amd/amddefs.h>
#include <cpu/amd/common/common.h>
#include <cpu/amd/msr.h>
#include <cpu/x86/lapic.h>
#include <drivers/amd/hypertransport/AsPsDefs.h>
#include <drivers/amd/hypertransport/ht_wrapper.h>
#include <device/pci_ops.h>
#include <stdint.h>
#include <stdlib.h>
#include <timer.h>

#include "family_10h_15h.h"
#include "fidvid.h"

#if CONFIG(SET_FIDVID_DEBUG)
#define print_debug_fv(str, val)		printk(BIOS_DEBUG, "%s%x\n", str, val)
#define print_debug_fv_8(str, val)		printk(BIOS_DEBUG, "%s%02x\n", str, val)
#define print_debug_fv_64(str, val, val2)	printk(BIOS_DEBUG, "%s%x%x\n", str, val, val2)
#else
#define print_debug_fv(str, val)		do { } while(0)
#define print_debug_fv_8(str, val)		do { } while(0)
#define print_debug_fv_64(str, val, val2)	do { } while(0)
#endif

struct fidvid_st {
	u32 common_fid;
};

static void enable_fid_change(u8 fid)
{
	u32 dword;
	u32 nodes;
	int i;

	nodes = get_nodes();

	for (i = 0; i < nodes; i++) {
		dword = pci_read_config32(NODE_PCI(i, 3), 0xd4);
		dword &= ~0x1F;
		dword |= (u32) fid & 0x1F;
		dword |= 1 << 5;	// enable
		pci_write_config32(NODE_PCI(i, 3), 0xd4, dword);
		printk(BIOS_DEBUG, "FID Change Node:%02x, F3xD4: %08x\n", i, dword);
	}
}

static void applyBoostFIDOffset(uint32_t nodeid)
{
	// BKDG 2.4.2.8
	// Fam10h revision E only, but E is apparently not supported yet, therefore untested
	// Check if CPB capable and num cores -1 is 5
	if ((cpuid_edx(0x80000007) & CPB_MASK) && ((cpuid_ecx(0x80000008) & NC_MASK) == 5)) {
		u32 core =  get_node_core_id_x().coreid;
		u32 asymetricBoostThisCore = pci_read_config32(NODE_PCI(nodeid, 3), 0x10C);
		asymetricBoostThisCore =  (asymetricBoostThisCore >> (core * 2)) & 3;
		msr_t msr =  rdmsr(PSTATE_0_MSR);
		u32 cpuFid = msr.lo & PS_CPU_FID_MASK;
		cpuFid = cpuFid + asymetricBoostThisCore;
		msr.lo &= ~PS_CPU_FID_MASK;
		msr.lo |= cpuFid;
	} else if (is_fam15h()) {
		uint32_t dword = pci_read_config32(NODE_PCI(nodeid, 4), 0x15c);
		uint8_t boost_count = (dword >> 2) & 0x7;
		if (boost_count > 0) {
			/* Enable boost */
			dword &= ~0x3;
			dword |= 0x1;
			pci_write_config32(NODE_PCI(nodeid, 4), 0x15c, dword);
		}
	}
}

static void enableNbPState1(pci_devfn_t dev)
{
	uint64_t cpuRev =  get_logical_CPUID(0xFF);
	if (cpuRev & AMD_FAM10_C3) {
		u32 nbPState = (pci_read_config32(dev, 0x1F0) & NB_PSTATE_MASK);
		if ( nbPState){
			u32 nbVid1 = (pci_read_config32(dev, 0x1F4) & NB_VID1_MASK);
			nbVid1 >>= NB_VID1_SHIFT;
			u32 i;
			for (i = nbPState; i < NM_PS_REG; i++) {
				msr_t msr =  rdmsr(PSTATE_0_MSR + i);
				if (msr.hi &  PS_EN_MASK ) {
					msr.hi |= NB_DID_M_ON;
					msr.lo &= NB_VID_MASK_OFF;
					msr.lo |= ( nbVid1 << NB_VID_POS);
					wrmsr(PSTATE_0_MSR + i, msr);
				}
			}
		}
	}
}

static u8 setPStateMaxVal(pci_devfn_t dev)
{
	u8 i, maxpstate = 0;

	for (i = 0; i < NM_PS_REG; i++) {
		msr_t msr = rdmsr(PSTATE_0_MSR + i);
		if (msr.hi & PS_IDD_VALUE_MASK) {
			msr.hi |= PS_EN_MASK;
			wrmsr(PSTATE_0_MSR + i, msr);
		}
		if (msr.hi & PS_EN_MASK) {
			maxpstate = i;
		}
	}

	//FIXME: CPTC2 and HTC_REG should get max per node, not per core ?
	u32 reg = pci_read_config32(dev, CPTC2);
	reg &= PS_MAX_VAL_MASK;
	reg |= (maxpstate << PS_MAX_VAL_POS);
	pci_write_config32(dev, CPTC2,reg);
	return maxpstate;
}

static void dualPlaneOnly(pci_devfn_t dev)
{
	// BKDG 2.4.2.7
	uint64_t cpuRev = get_logical_CPUID(0xff);
	if ((get_processor_package_type() == AMD_PKGTYPE_AM3_2r2) &&
	    (cpuRev & (AMD_DR_Cx | AMD_DR_Ex))) {
		if ((pci_read_config32(dev, 0x1fc) & DUAL_PLANE_ONLY_MASK) &&
		    (pci_read_config32(dev, 0xa0) & PVI_MODE)) {
			if (cpuid_edx(0x80000007) & CPB_MASK) {
				// revision E only, but E is apparently not supported yet, therefore untested
				msr_t minPstate = rdmsr(PSTATE_1_MSR);
				wrmsr(PSTATE_1_MSR, rdmsr(PSTATE_4_MSR));
				wrmsr(PSTATE_4_MSR, minPstate);
			} else {
				msr_t msr;
				msr.lo=0; msr.hi=0;
				wrmsr(PSTATE_0_MSR, rdmsr(PSTATE_4_MSR));
				wrmsr(PSTATE_4_MSR, msr);
			}

			//FIXME: CPTC2 and HTC_REG should get max per node, not per core ?
			u8 maxpstate = setPStateMaxVal(dev);

			u32 reg = pci_read_config32(dev, HTC_REG);
			reg &= HTC_PS_LMT_MASK;
			reg |= (maxpstate << PS_LIMIT_POS);
			pci_write_config32(dev, HTC_REG,reg);
		}
	}
}

static int vidTo100uV(u8 vid)
{
	// returns voltage corresponding to vid in tenths of mV, i.e. hundreds of uV
	// BKDG #31116 rev 3.48 2.4.1.6
	int voltage;
	if (vid >= 0x7c) {
		voltage = 0;
	} else {
		voltage = (15500 - (125*vid));
	}
	return voltage;
}

static void setVSRamp(pci_devfn_t dev)
{
	/* BKDG r31116 2010-04-22  2.4.1.7 step b F3xD8[VSRampTime]
	 * If this field accepts 8 values between 10 and 500 us why
	 * does page 324 say "BIOS should set this field to 001b."
	 * (20 us) ?
	 * Shouldn't it depend on the voltage regulators, mainboard
	 * or something ?
	 */
	u32 dword;
	dword = pci_read_config32(dev, 0xd8);
	dword &= VSRAMP_MASK;
	dword |= VSRAMP_VALUE;
	pci_write_config32(dev, 0xd8, dword);
}

static void recalculateVsSlamTimeSettingOnCorePre(pci_devfn_t dev)
{
	u8 pviModeFlag;
	u8 highVoltageVid, lowVoltageVid, bValue;
	u16 minimumSlamTime;
	/* Reg settings scaled by 100 */
	u16 vSlamTimes[7] = { 1000, 2000, 3000, 4000, 6000, 10000, 20000 };
	u32 dtemp;
	msr_t msr;

	/* This function calculates the VsSlamTime using the range of possible
	 * voltages instead of a hardcoded 200us.
	 * Note: his function is called only from prep_fid_change,
	 * and that from init_cpus.c finalize_node_setup()
	 * (after set AMD MSRs and init ht )
	 */

	/* BKDG r31116 2010-04-22  2.4.1.7 step b F3xD8[VSSlamTime] */
	/* Calculate Slam Time
	 * Vslam = (mobileCPU?0.2:0.4)us/mV * (Vp0 - (lowest out of Vpmin or Valt)) mV
	 * In our case, we will scale the values by 100 to avoid
	 * decimals.
	 */

	/* Determine if this is a PVI or SVI system */
	if (is_fam15h()) {
		pviModeFlag = 0;
	} else {
		dtemp = pci_read_config32(dev, 0xa0);

		if (dtemp & PVI_MODE)
			pviModeFlag = 1;
		else
			pviModeFlag = 0;
	}

	/* Get P0's voltage */
	/* MSRC001_00[68:64] are not programmed yet when called from
	 * prep_fid_change, one might use F4x1[F0:E0] instead, but
	 * theoretically MSRC001_00[68:64] are equal to them after
	 * reset.
	 */
	msr = rdmsr(PSTATE_0_MSR);
	highVoltageVid = (u8) ((msr.lo >> PS_CPU_VID_SHFT) & 0x7F);
	if (!(msr.hi & 0x80000000)) {
	    printk(BIOS_ERR,"P-state info in MSRC001_0064 is invalid !!!\n");
	    highVoltageVid = (u8) ((pci_read_config32(dev, 0x1E0)
				     >> PS_CPU_VID_SHFT) & 0x7F);
	}

	/* If SVI, we only care about CPU VID.
	 * If PVI, determine the higher voltage b/t NB and CPU
	 */
	if (pviModeFlag) {
		bValue = (u8) ((msr.lo >> PS_NB_VID_SHFT) & 0x7F);
		if (highVoltageVid > bValue)
			highVoltageVid = bValue;
	}

	/* Get PSmax's index */
	msr = rdmsr(PS_LIM_REG);
	bValue = (u8) ((msr.lo >> PS_MAX_VAL_SHFT) & BIT_MASK_3);

	/* Get PSmax's VID */
	msr = rdmsr(PSTATE_0_MSR + bValue);
	lowVoltageVid = (u8) ((msr.lo >> PS_CPU_VID_SHFT) & 0x7F);
	if (!(msr.hi & 0x80000000)) {
		printk(BIOS_ERR, "P-state info in MSR%8x is invalid !!!\n",
			PSTATE_0_MSR + bValue);
		lowVoltageVid = (u8) ((pci_read_config32(dev, 0x1E0+(bValue*4))
				     >> PS_CPU_VID_SHFT) & 0x7F);
	}

	/* If SVI, we only care about CPU VID.
	 * If PVI, determine the higher voltage b/t NB and CPU
	 * BKDG 2.4.1.7 (a)
	 */
	if (pviModeFlag) {
		bValue = (u8) ((msr.lo >> PS_NB_VID_SHFT) & 0x7F);
		if (lowVoltageVid > bValue)
			lowVoltageVid = bValue;
	}

	/* Get AltVID */
	dtemp = pci_read_config32(dev, 0xdc);
	bValue = (u8) (dtemp & BIT_MASK_7);

	/* Use the VID with the lowest voltage (higher VID) */
	if (lowVoltageVid < bValue)
		lowVoltageVid = bValue;

	u8 mobileFlag = get_platform_type() & AMD_PTYPE_MOB;
	minimumSlamTime =  (mobileFlag?2:4) * (vidTo100uV(highVoltageVid) - vidTo100uV(lowVoltageVid)); /* * 0.01 us */


	/* Now round up to nearest register setting.
	 * Note that if we don't find a value, we
	 * will fall through to a value of 7
	 */
	for (bValue = 0; bValue < 7; bValue++) {
		if (minimumSlamTime <= vSlamTimes[bValue])
			break;
	}

	/* Apply the value */
	dtemp = pci_read_config32(dev, 0xD8);
	dtemp &= VSSLAM_MASK;
	dtemp |= bValue;
	pci_write_config32(dev, 0xd8, dtemp);
}

static u32 nb_clk_did(uint8_t node, uint64_t cpuRev, uint8_t procPkg)
{
	uint8_t link0isGen3 = 0;
	uint8_t offset;

	if (AMD_CpuFindCapability(node, 0, &offset)) {
		link0isGen3 = (AMD_checkLinkType(node, offset) & HTPHY_LINKTYPE_HT3);
	}
	/* FIXME: NB_CLKDID should be 101b for AMD_DA_C2 in package
	   S1g3 in link Gen3 mode, but I don't know how to tell
	   package S1g3 from S1g4 */
	if ((cpuRev & AMD_DA_C2) && (procPkg & AMD_PKGTYPE_S1gX) && link0isGen3) {
		return 5; /* divide clk by 128*/
	} else {
		return 4; /* divide clk by 16 */
	}
}

static u32 power_up_down(int node, u8 procPkg)
{
	uint32_t dword = 0;

	/* from CPU rev guide #41322 rev 3.74 June 2010 Table 26 */
	u8 singleLinkFlag = ((procPkg == AMD_PKGTYPE_AM3_2r2)
			  || (procPkg == AMD_PKGTYPE_S1gX)
			  || (procPkg == AMD_PKGTYPE_ASB2));

	if (singleLinkFlag) {
		/*
		 * PowerStepUp=01000b - 50nS
		 * PowerStepDown=01000b - 50ns
		 */
		dword |= PW_STP_UP50 | PW_STP_DN50;
	} else {
		uint32_t dispRefModeEn = (pci_read_config32(NODE_PCI(node,0),0x68) >> 24) & 1;
		uint32_t isocEn = 0;
		int j;
		for (j=0; (j<4) && (!isocEn); j++ ) {
			u8 offset;
			if (AMD_CpuFindCapability(node, j, &offset)) {
				isocEn = (pci_read_config32(NODE_PCI(node,0),offset+4) >>12) & 1;
			}
		}

		if (is_fam15h()) {
			/* Family 15h always uses 100ns for multilink processors */
			dword |= PW_STP_UP100 | PW_STP_DN100;
		} else if (dispRefModeEn || isocEn) {
			dword |= PW_STP_UP50 | PW_STP_DN50;
		} else {
			/* get number of cores for PowerStepUp & PowerStepDown in server
			 * 1 core - 400nS  - 0000b
			 * 2 cores - 200nS - 0010b
			 * 3 cores - 133nS -> 100nS - 0011b
			 * 4 cores - 100nS - 0011b
			 */
			switch (get_core_num_in_bsp(node)) {
			case 0:
				dword |= PW_STP_UP400 | PW_STP_DN400;
				break;
			case 1:
			case 2:
				dword |= PW_STP_UP200 | PW_STP_DN200;
				break;
			case 3:
				dword |= PW_STP_UP100 | PW_STP_DN100;
				break;
			default:
				dword |= PW_STP_UP100 | PW_STP_DN100;
				break;
			}
		}
	}

	return dword;
}

static void config_clk_power_ctrl_reg0(uint8_t node, uint64_t cpuRev, uint8_t procPkg) {

	pci_devfn_t dev = NODE_PCI(node, 3);

	/* Program fields in Clock Power/Control register0 (F3xD4) */

	/* set F3xD4 Clock Power/Timing Control 0 Register
	 * NbClkDidApplyAll=1b
	 * NbClkDid=100b or 101b
	 * PowerStepUp= "platform dependent"
	 * PowerStepDown= "platform dependent"
	 * LinkPllLink=01b
	 * ClkRampHystCtl=HW default
	 * ClkRampHystSel=1111b
	 */
	uint32_t dword= pci_read_config32(dev, 0xd4);
	dword &= CPTC0_MASK;
	dword |= NB_CLKDID_ALL | LNK_PLL_LOCK | CLK_RAMP_HYST_SEL_VAL;
	dword |= (nb_clk_did(node,cpuRev,procPkg) <<  NB_CLKDID_SHIFT);

	dword |= power_up_down(node, procPkg);

	pci_write_config32(dev, 0xd4, dword);

}

static void config_power_ctrl_misc_reg(pci_devfn_t dev, uint64_t cpuRev,
		uint8_t procPkg)
{
	/* check PVI/SVI */
	uint32_t dword = pci_read_config32(dev, 0xa0);

	/* BKDG r31116 2010-04-22  2.4.1.7 step b F3xA0[VSSlamVidMod] */
	/* PllLockTime and PsiVidEn set in ruleset in defaults.h */
	if (dword & PVI_MODE) {	/* PVI */
		/* set slamVidMode to 0 for PVI */
		dword &= VID_SLAM_OFF;
	} else {	/* SVI */
		/* set slamVidMode to 1 for SVI */
		dword |= VID_SLAM_ON;
	}
	/* set the rest of A0 since we're at it... */

	if (cpuRev & (AMD_DA_Cx | AMD_RB_C3 )) {
		dword |= NB_PSTATE_FORCE_ON;
	} // else should we clear it ?


	if ((procPkg == AMD_PKGTYPE_G34) || (procPkg == AMD_PKGTYPE_C32) ) {
		dword |= BP_INS_TRI_EN_ON;
	}

	   /* TODO: look into C1E state and F3xA0[IdleExitEn]*/
	#if CONFIG(SVI_HIGH_FREQ)
	if (cpuRev & AMD_FAM10_C3) {
		dword |= SVI_HIGH_FREQ_ON;
	}
	#endif
	pci_write_config32(dev, 0xa0, dword);
}

static void config_nb_syn_ptr_adj(pci_devfn_t dev, uint64_t cpuRev)
{
	/* Note the following settings are additional from the ported
	 * function setFidVidRegs()
	 */
	/* adjust FIFO between nb and core clocks to max allowed
	   values (min latency) */
	uint32_t nbPstate = pci_read_config32(dev,0x1f0) & NB_PSTATE_MASK;
	uint8_t nbSynPtrAdj;
	if ((cpuRev & (AMD_DR_Bx | AMD_DA_Cx | AMD_FAM15_ALL) )
		|| ((cpuRev & AMD_RB_C3) && (nbPstate != 0))) {
		nbSynPtrAdj = 5;
	} else {
		nbSynPtrAdj = 6;
	}

	uint32_t dword = pci_read_config32(dev, 0xdc);
	dword &= ~NB_SYN_PTR_ADJ_MASK;
	dword |= nbSynPtrAdj << NB_SYN_PTR_ADJ_POS;
	/* NbsynPtrAdj set to 5 or 6 per BKDG (needs reset) */
	pci_write_config32(dev, 0xdc, dword);
}

static void config_acpi_pwr_state_ctrl_regs(pci_devfn_t dev, uint64_t cpuRev,
		uint8_t procPkg)
{
	if (is_fam15h()) {
		/* Family 15h BKDG Rev. 3.14 D18F3x80 recommended settings */
		pci_write_config32(dev, 0x80, 0xe20be281);

		/* Family 15h BKDG Rev. 3.14 D18F3x84 recommended settings */
		pci_write_config32(dev, 0x84, 0x01e200e2);
	} else {
		/* step 1, chapter 2.4.2.6 of AMD Fam 10 BKDG #31116 Rev 3.48 22.4.2010 */
		uint32_t dword;
		uint32_t c1= 1;
		if (cpuRev & (AMD_DR_Bx)) {
			// will coreboot ever enable cache scrubbing ?
			// if it does, will it be enough to check the current state
			// or should we configure for what we'll set up later ?
			dword = pci_read_config32(dev, 0x58);
			uint32_t scrubbingCache = dword &
						( (0x1f << 16) // DCacheScrub
						| (0x1f << 8) ); // L2Scrub
			if (scrubbingCache) {
				c1 = 0x80;
			} else {
				c1 = 0xa0;
			}
		} else { // rev C or later
			// same doubt as cache scrubbing: ok to check current state ?
			dword = pci_read_config32(dev, 0xdc);
			uint32_t cacheFlushOnHalt = dword & (7 << 16);
			if (!cacheFlushOnHalt) {
				c1 = 0x80;
			}
		}
		dword = (c1 << 24) | (0xe641e6);
		pci_write_config32(dev, 0x84, dword);

		/* FIXME: BKDG Table 100 says if the link is at a Gen1
		* frequency and the chipset does not support a 10us minimum LDTSTOP
		* assertion time, then { If ASB2 && SVI then smaf001 = F6h else
		* smaf001=87h. } else ...  I hardly know what it means or how to check
		* it from here, so I bluntly assume it is false and code here the else,
		* which is easier
		*/

		uint32_t smaf001 = 0xE6;
		if (cpuRev & AMD_DR_Bx ) {
			smaf001 = 0xA6;
		} else {
			if (CONFIG(SVI_HIGH_FREQ)) {
				if (cpuRev & (AMD_RB_C3 | AMD_DA_C3)) {
					smaf001 = 0xF6;
				}
			}
		}
		uint32_t fidvidChange = 0;
		if (((cpuRev & AMD_DA_Cx) && (procPkg & AMD_PKGTYPE_S1gX))
			|| (cpuRev & AMD_RB_C3) ) {
				fidvidChange = 0x0b;
		}
		dword = (0xE6 << 24) | (fidvidChange << 16)
				| (smaf001 << 8) | 0x81;
		pci_write_config32(dev, 0x80, dword);
	}
}

void prep_fid_change(void)
{
	u32 dword;
	u32 nodes;
	pci_devfn_t dev;
	int i;

	/* This needs to be run before any Pstate changes are requested */

	nodes = get_nodes();

	for (i = 0; i < nodes; i++) {
		printk(BIOS_DEBUG, "Prep FID/VID Node:%02x\n", i);
		dev = NODE_PCI(i, 3);
		uint64_t cpuRev = get_logical_CPUID(0xFF);
		u8 procPkg =  get_processor_package_type();

		setVSRamp(dev);
		/* BKDG r31116 2010-04-22  2.4.1.7 step b F3xD8[VSSlamTime] */
		/* Figure out the value for VsSlamTime and program it */
		recalculateVsSlamTimeSettingOnCorePre(dev);

		config_clk_power_ctrl_reg0(i,cpuRev,procPkg);

		config_power_ctrl_misc_reg(dev,cpuRev,procPkg);
		config_nb_syn_ptr_adj(dev,cpuRev);

		config_acpi_pwr_state_ctrl_regs(dev,cpuRev,procPkg);

		dword = pci_read_config32(dev, 0x80);
		printk(BIOS_DEBUG, "  F3x80: %08x\n", dword);
		dword = pci_read_config32(dev, 0x84);
		printk(BIOS_DEBUG, "  F3x84: %08x\n", dword);
		dword = pci_read_config32(dev, 0xd4);
		printk(BIOS_DEBUG, "  F3xD4: %08x\n", dword);
		dword = pci_read_config32(dev, 0xd8);
		printk(BIOS_DEBUG, "  F3xD8: %08x\n", dword);
		dword = pci_read_config32(dev, 0xdc);
		printk(BIOS_DEBUG, "  F3xDC: %08x\n", dword);
	}
}

static void waitCurrentPstate(u32 target_pstate)
{
	msr_t pstate_msr = rdmsr(PS_STS_REG);
	struct stopwatch sw;

	stopwatch_init_msecs_expire(&sw, 100);

	do {
		pstate_msr = rdmsr(PS_STS_REG);
		if (stopwatch_expired(&sw)) {
			msr_t limit_msr = rdmsr(PS_LIM_REG);
			printk(BIOS_ERR, "*** APIC ID %02x: timed out waiting for P-state %01x."
				 " Current P-state %01x P-state current limit"
				 " MSRC001_0061=%08x %08x\n",
				cpuid_ebx(0x00000001) >> 24, target_pstate, pstate_msr.lo,
				limit_msr.hi, limit_msr.lo);
			break;
		}
	} while ((pstate_msr.lo != target_pstate));

	do { // should we just go on instead ?
		pstate_msr = rdmsr(PS_STS_REG);
	} while (pstate_msr.lo != target_pstate);
}

static void set_pstate(u32 nonBoostedPState) {
	msr_t msr;
	uint8_t skip_wait;

	// Transition P0 for calling core.
	msr = rdmsr(PS_CTL_REG);

	msr.lo = nonBoostedPState;
	wrmsr(PS_CTL_REG, msr);

	if (is_fam15h()) {
		/* Do not wait for the first (even) set of cores to transition on Family 15h systems */
		if ((cpuid_ebx(0x00000001) & 0x01000000))
			skip_wait = 0;
		else
			skip_wait = 1;
	} else {
		skip_wait = 0;
	}

	if (!skip_wait) {
		/* Wait for core to transition to P0 */
		waitCurrentPstate(nonBoostedPState);
	}
}

static void UpdateSinglePlaneNbVid(void)
{
	u32 nbVid, cpuVid;
	u8 i;
	msr_t msr;

	/* copy higher voltage (lower VID) of NBVID & CPUVID to both */
	for (i = 0; i < 5; i++) {
		msr = rdmsr(PSTATE_0_MSR + i);
		nbVid = (msr.lo & PS_CPU_VID_M_ON) >> PS_CPU_VID_SHFT;
		cpuVid = (msr.lo & PS_NB_VID_M_ON) >> PS_NB_VID_SHFT;

		if (nbVid != cpuVid) {
			if (nbVid > cpuVid)
				nbVid = cpuVid;

			msr.lo = msr.lo & PS_BOTH_VID_OFF;
			msr.lo = msr.lo | (u32) ((nbVid) << PS_NB_VID_SHFT);
			msr.lo = msr.lo | (u32) ((nbVid) << PS_CPU_VID_SHFT);
			wrmsr(PSTATE_0_MSR + i, msr);
		}
	}
}

static void fixPsNbVidBeforeWR(u32 newNbVid, u32 coreid, u32 dev, u8 pviMode)
{
	msr_t msr;
	u8 startup_pstate;

	/* This function sets NbVid before the warm reset.
	 *       Get StartupPstate from MSRC001_0071.
	 *       Read Pstate register pointed by [StartupPstate].
	 *       and copy its content to P0 and P1 registers.
	 *       Copy newNbVid to P0[NbVid].
	 *       transition to P1 on all cores,
	 *       then transition to P0 on core 0.
	 *       Wait for MSRC001_0063[CurPstate] = 000b on core 0.
	 * see BKDG rev 3.48  2.4.2.9.1 BIOS NB COF and VID Configuration
	 *			      for SVI and Single-Plane PVI Systems
	 */

	msr = rdmsr(MSR_COFVID_STS);
	startup_pstate = (msr.hi >> (32 - 32)) & 0x07;

	/* Copy startup pstate to P1 and P0 MSRs. Set the maxvid for
	 * this node in P0.  Then transition to P1 for corex and P0
	 * for core0.  These setting will be cleared by the warm reset
	 */
	msr = rdmsr(PSTATE_0_MSR + startup_pstate);
	wrmsr(PSTATE_1_MSR, msr);
	wrmsr(PSTATE_0_MSR, msr);

	/* missing step 2 from BDKG , F3xDC[PstateMaxVal] =
	 * max(1,F3xDC[PstateMaxVal] ) because it would take
	 * synchronization between cores and we don't think
	 * PstatMaxVal is going to be 0 on cold reset anyway ?
	 */
	if (!(pci_read_config32(dev, 0xdc) & (~PS_MAX_VAL_MASK))) {
	   printk(BIOS_ERR,"F3xDC[PstateMaxVal] is zero. Northbridge voltage setting will fail. fixPsNbVidBeforeWR in fidvid.c needs fixing. See AMD # 31116 rev 3.48 BKDG 2.4.2.9.1\n");
	};

	msr.lo &= ~0xFE000000;	// clear nbvid
	msr.lo |= (newNbVid << 25);
	wrmsr(PSTATE_0_MSR, msr);

	if (pviMode)/* single plane*/
		UpdateSinglePlaneNbVid();

	// Transition to P1 for all APs and P0 for core0.
	set_pstate(1);

	if (coreid == 0)
		set_pstate(0);

	/* FIXME: missing step 7 (restore PstateMax to 0 if needed) because
	 * we skipped step 2
	 */

}

static u32 needs_NB_COF_VID_update(void)
{
	u8 nb_cof_vid_update;
	u8 nodes;
	u8 i;

	if (is_fam15h())
		return 0;

	/* If any node has nb_cof_vid_update set all nodes need an update. */
	nodes = get_nodes();
	nb_cof_vid_update = 0;
	for (i = 0; i < nodes; i++) {
		uint64_t cpuRev = get_logical_CPUID(i);
		u32 nbCofVidUpdateDefined = (cpuRev & (AMD_FAM10_LT_D));
		if (nbCofVidUpdateDefined
		    && (pci_read_config32(NODE_PCI(i, 3), 0x1FC)
			& NB_COF_VID_UPDATE_MASK)) {
			nb_cof_vid_update = 1;
			break;
		}
	}
	return nb_cof_vid_update;
}

static u32 init_fidvid_core(u32 nodeid, u32 coreid)
{
	pci_devfn_t dev;
	u32 vid_max;
	u32 fid_max = 0;
	u8 nb_cof_vid_update = needs_NB_COF_VID_update();
	u8 pvimode;
	u32 reg1fc;

	/* Steps 1-6 of BIOS NB COF and VID Configuration
	 * for SVI and Single-Plane PVI Systems. BKDG 2.4.2.9 #31116 rev 3.48
	 */
	dev = NODE_PCI(nodeid, 3);
	if (is_fam15h())
		pvimode = 0;
	else
		pvimode = pci_read_config32(dev, PW_CTL_MISC) & PVI_MODE;
	reg1fc = pci_read_config32(dev, 0x1FC);

	if (nb_cof_vid_update) {
		vid_max = (reg1fc &  SINGLE_PLANE_NB_VID_MASK ) >>  SINGLE_PLANE_NB_VID_SHIFT;
		fid_max = (reg1fc &  SINGLE_PLANE_NB_FID_MASK ) >>  SINGLE_PLANE_NB_FID_SHIFT;

		if (!pvimode) { /* SVI, dual power plane */
			vid_max = vid_max - ((reg1fc &  DUAL_PLANE_NB_VID_OFF_MASK ) >>  DUAL_PLANE_NB_VID_SHIFT );
			fid_max = fid_max +  ((reg1fc &  DUAL_PLANE_NB_FID_OFF_MASK ) >>  DUAL_PLANE_NB_FID_SHIFT );
		}
		/* write newNbVid to P-state Reg's NbVid always if NbVidUpdatedAll=1 */
		fixPsNbVidBeforeWR(vid_max, coreid, dev, pvimode);

		/* fid setup is handled by the BSP at the end. */

	} else {	/* ! nb_cof_vid_update */
		/* Use max values */
		if (pvimode)
			UpdateSinglePlaneNbVid();
	}

	return ((nb_cof_vid_update << 16) | (fid_max << 8));

}

void init_fidvid_ap(u32 apicid, u32 nodeid, u32 coreid)
{
	u32 send;

	printk(BIOS_DEBUG, "FIDVID on AP: %02x\n", apicid);

	send = init_fidvid_core(nodeid, coreid);
	send |= (apicid << 24);	// ap apicid

	// Send signal to BSP about this AP max fid
	// This also indicates this AP is ready for warm reset (if required).
	lapic_write(LAPIC_MSG_REG, send | F10_APSTATE_RESET);
}

static u32 calc_common_fid(u32 fid_packed, u32 fid_packed_new)
{
	u32 fidmax;
	u32 fidmax_new;

	fidmax = (fid_packed >> 8) & 0xFF;

	fidmax_new = (fid_packed_new >> 8) & 0xFF;

	if (fidmax > fidmax_new) {
		fidmax = fidmax_new;
	}

	fid_packed &= 0xFF << 16;
	fid_packed |= (fidmax << 8);
	fid_packed |= fid_packed_new & (0xFF << 16);	// set nb_cof_vid_update

	return fid_packed;
}

static void init_fidvid_bsp_stage1(u32 ap_apicid, void *gp)
{
	u32 readback = 0;
	u32 timeout = 1;

	struct fidvid_st *fvp = gp;
	int loop;

	print_debug_fv("Wait for AP stage 1: ap_apicid = ", ap_apicid);

	loop = 100000;
	while (--loop > 0) {
		if (lapic_remote_read(ap_apicid, LAPIC_MSG_REG, &readback) != 0)
			continue;
		if (((readback & 0x3f) == F10_APSTATE_RESET)
			|| (is_fam15h() && ((readback & 0x3f) == F10_APSTATE_ASLEEP))) {
			timeout = 0;
			break;	/* target ap is in stage 1 */
		}
	}

	if (timeout) {
		printk(BIOS_DEBUG, "%s: timed out reading from ap %02x\n",
		       __func__, ap_apicid);
		return;
	}

	print_debug_fv("\treadback = ", readback);

	fvp->common_fid = calc_common_fid(fvp->common_fid, readback);

	print_debug_fv("\tcommon_fid(packed) = ", fvp->common_fid);

}

static void fixPsNbVidAfterWR(u32 newNbVid, u8 NbVidUpdatedAll,u8 pviMode)
{
	msr_t msr;
	u8 i;
	u8 StartupPstate;

	/* BKDG 2.4.2.9.1 11-12
	 * This function copies newNbVid to NbVid bits in P-state
	 * Registers[4:0] if its NbDid bit=0, and IddValue!=0 in case of
	 * NbVidUpdatedAll =0 or copies newNbVid to NbVid bits in
	 * P-state Registers[4:0] if its IddValue!=0 in case of
	 * NbVidUpdatedAll=1. Then transition to StartPstate.
	 */

	/* write newNbVid to P-state Reg's NbVid if its NbDid=0 */
	for (i = 0; i < 5; i++) {
		msr = rdmsr(PSTATE_0_MSR + i);
		/*  NbDid (bit 22 of P-state Reg) == 0  or NbVidUpdatedAll = 1 */
		if (   (msr.hi & PS_IDD_VALUE_MASK)
		    && (msr.hi & PS_EN_MASK)
		    &&(((msr.lo & PS_NB_DID_MASK) == 0) || NbVidUpdatedAll)) {
			msr.lo &= PS_NB_VID_M_OFF;
			msr.lo |= (newNbVid & 0x7F) << PS_NB_VID_SHFT;
			wrmsr(PSTATE_0_MSR + i, msr);
		}
	}

	/* Not documented. Would overwrite Nb_Vids just copied
	 * should we just update cpu_vid or nothing at all ?
	 */
	if (pviMode) { //single plane
		UpdateSinglePlaneNbVid();
	}
	/* For each core in the system, transition all cores to StartupPstate */
	msr = rdmsr(MSR_COFVID_STS);
	StartupPstate = msr.hi & 0x07;

	/* Set and wait for StartupPstate to set. */
	set_pstate(StartupPstate);

}

void init_fidvid_stage2(u32 apicid, u32 nodeid)
{
	msr_t msr;
	u32 reg1fc;
	u32 dtemp;
	u32 nbvid;
	u8 nb_cof_vid_update = needs_NB_COF_VID_update();
	u8 NbVidUpdateAll;
	u8 pvimode;

	/* After warm reset finish the fid/vid setup for all cores. */

	/* If any node has nb_cof_vid_update set all nodes need an update. */
	if (is_fam15h())
		pvimode = 0;
	else
		pvimode = (pci_read_config32(NODE_PCI(nodeid, 3), 0xA0) >> 8) & 1;
	reg1fc = pci_read_config32(NODE_PCI(nodeid, 3), 0x1FC);
	nbvid = (reg1fc >> 7) & 0x7F;
	NbVidUpdateAll = (reg1fc >> 1) & 1;

	if (nb_cof_vid_update) {
		if (!pvimode) {	/* SVI */
			nbvid = nbvid - ((reg1fc >> 17) & 0x1F);
		}
		/* write newNbVid to P-state Reg's NbVid if its NbDid=0 */
		fixPsNbVidAfterWR(nbvid, NbVidUpdateAll,pvimode);
	} else {		/* !nb_cof_vid_update */
		if (pvimode)
			UpdateSinglePlaneNbVid();
	}
	dtemp = pci_read_config32(NODE_PCI(nodeid, 3), 0xA0);
	dtemp &= PLLLOCK_OFF;
	dtemp |= PLLLOCK_DFT_L;
	pci_write_config32(NODE_PCI(nodeid, 3), 0xA0, dtemp);

	dualPlaneOnly(NODE_PCI(nodeid, 3));
	applyBoostFIDOffset(nodeid);
	enableNbPState1(NODE_PCI(nodeid, 3));

	/* Enable P0 on all cores for best performance.
	 * Linux can slow them down later if need be.
	 * It is safe since they will be in C1 halt
	 * most of the time anyway.
	 */
	set_pstate(0);

	if (!is_fam15h()) {
		/* Set TSC to tick at the P0 ndfid rate */
		msr = rdmsr(HWCR_MSR);
		msr.lo |= 1 << 24;
		wrmsr(HWCR_MSR, msr);
	}
}

struct ap_apicid_st {
	u32 num;
	// it could use 256 bytes for 64 node quad core system
	u8 apicid[NODE_NUMS * 4];
};

static void store_ap_apicid(unsigned int ap_apicid, void *gp)
{
	struct ap_apicid_st *p = gp;

	p->apicid[p->num++] = ap_apicid;

}

int init_fidvid_bsp(u32 bsp_apicid, u32 nodes)
{

	struct ap_apicid_st ap_apicidx;
	u32 i;
	struct fidvid_st fv;

	printk(BIOS_DEBUG, "FIDVID on BSP, APIC_id: %02x\n", bsp_apicid);

	/* Steps 1-6 of BIOS NB COF and VID Configuration
	 * for SVI and Single-Plane PVI Systems.
	 */
	fv.common_fid = init_fidvid_core(0, 0);

	print_debug_fv("BSP fid = ", fv.common_fid);

	if (CONFIG(SET_FIDVID_STORE_AP_APICID_AT_FIRST) && !CONFIG(SET_FIDVID_CORE0_ONLY)) {
		/* For all APs (We know the APIC ID of all APs even when the APIC ID
		 * is lifted) remote read from AP LAPIC_MSG_REG about max fid.
		 * Then calculate the common max fid that can be used for all
		 * APs and BSP
		 */
		ap_apicidx.num = 0;

		for_each_ap(bsp_apicid, CONFIG_SET_FIDVID_CORE_RANGE, -1, store_ap_apicid, &ap_apicidx);

		for (i = 0; i < ap_apicidx.num; i++) {
			init_fidvid_bsp_stage1(ap_apicidx.apicid[i], &fv);
		}
	} else {
		for_each_ap(bsp_apicid, CONFIG(SET_FIDVID_CORE0_ONLY), -1,
			    init_fidvid_bsp_stage1, &fv);
	}

	print_debug_fv("common_fid = ", fv.common_fid);

	if (fv.common_fid & (1 << 16)) {	/* check nb_cof_vid_update */

		// Enable the common fid and other settings.
		enable_fid_change((fv.common_fid >> 8) & 0x1F);

		// nbfid change need warm reset, so reset at first
		return 1;
	}

	return 0; // No FID/VID changes. Don't reset
}

/**
 * @file
 *
 * AMD Integrated Debug Debug_library Routines
 *
 * Contains AMD AGESA debug macros and library functions
 *
 * @xrefitem bom "File Content Label" "Release Content"
 * @e project:      AGESA
 * @e sub-project:  IDS
 * @e \$Revision: 63552 $   @e \$Date: 2011-12-26 19:46:05 -0600 (Mon, 26 Dec 2011) $
 */
/*****************************************************************************
 * Copyright (c) 2008 - 2012, Advanced Micro Devices, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Advanced Micro Devices, Inc. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ADVANCED MICRO DEVICES, INC. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************
 */

 /*----------------------------------------------------------------------------------------
 *                             M O D U L E S    U S E D
 *----------------------------------------------------------------------------------------
 */
#include "AGESA.h"
#include "Ids.h"
#include "IdsLib.h"
#include "amdlib.h"
#include "AMD.h"
#include "heapManager.h"
#include "cpuRegisters.h"
#include "cpuServices.h"
#include "GeneralServices.h"
#include "IdsDebugPrint.h"
#include "IdsDpHdtout.h"
#include "Filecode.h"
CODE_GROUP (G1_PEICC)
RDATA_GROUP (G1_PEICC)

#define FILECODE PROC_IDS_DEBUG_IDSDEBUG_FILECODE

/*--------------------------------------------------------------------------------------*/
/**
 *  IDS back-end code for AGESA_TESTPOINT
 *
 *  @param[in] TestPoint  Progress indicator value, see @ref AGESA_TP
 *  @param[in,out] StdHeader    The Pointer of AGESA Header
 *
 **/
/*--------------------------------------------------------------------------------------*/
VOID
IdsAgesaTestPoint (
  IN       AGESA_TP      TestPoint,
  IN OUT   AMD_CONFIG_PARAMS *StdHeader
  )
{
  LibAmdIoWrite (AccessWidth8, IDS_DEBUG_PORT, &TestPoint, StdHeader);
}

/**
 *  IDS Backend Function for Memory timeout control
 *
 *  This function is used to override Memory timeout control.
 *
 *  @param[in,out]   DataPtr      The Pointer of UINT8.
 *
 **/
VOID
IdsMemTimeOut (
  IN OUT   VOID *DataPtr
  )
{
}

/**
 *
 *  IDS Debug Function to check the sentinels are intact
 *
 * This function complete heap walk and check to be performed at any time.
 *
 * @param[in]      StdHeader         Config handle for library and services.
 *
 * @retval         TRUE     No error
 *
 **/
BOOLEAN
AmdHeapIntactCheck (
  IN       AMD_CONFIG_PARAMS *StdHeader
  )
{
  return TRUE;
}

/**
 * Check for CAR Corruption, the performance monitor number three programed to log the CAR Corruption.
 * Check to see if control register is enabled and then check the preformance counter and stop the system by executing
 * IDS_ERROR_TRAP if counter has any value other than zero.
 *
 *  @param[in,out] StdHeader    The Pointer of Standard Header.
 *
 *
 **/
VOID
IdsCarCorruptionCheck (
  IN OUT   AMD_CONFIG_PARAMS *StdHeader
  )
{

  UINT64 Perf_Msr;

  LibAmdMsrRead (MSR_PERF_CONTROL3, (UINT64*)&Perf_Msr, StdHeader);
  if ((Perf_Msr & PERF_RESERVE_BIT_MASK) == PERF_CAR_CORRUPTION_EVENT) {
    LibAmdMsrRead (MSR_PERF_COUNTER3, (UINT64*)&Perf_Msr, StdHeader);
    if ((Perf_Msr != 0)) {
      IDS_ERROR_TRAP;
    }
  }
}



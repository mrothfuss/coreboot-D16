/* $NoKeywords:$ */
/**
 * @file
 *
 * AMD Family_14 Microcode patch.
 *
 * Fam14 Microcode Patch rev 05000029 for 5010 or equivalent.
 *
 *
 */
/*
 ******************************************************************************
 *
 * Copyright (c) 2013, Advanced Micro Devices, Inc.
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
 *
 ******************************************************************************
 */
/*----------------------------------------------------------------------------------------
 *                             M O D U L E S    U S E D
 *----------------------------------------------------------------------------------------
 */
#include "AGESA.h"
#include "cpuRegisters.h"
#include "cpuEarlyInit.h"

/*----------------------------------------------------------------------------------------
 *                   D E F I N I T I O N S    A N D    M A C R O S
 *----------------------------------------------------------------------------------------
 */

/*----------------------------------------------------------------------------------------
 *                  T Y P E D E F S     A N D     S T R U C T U R E S
 *----------------------------------------------------------------------------------------
 */

// Encrypt Patch code 05000029 for 5010 and equivalent

CONST MICROCODE_PATCHES ROMDATA CpuF14MicrocodePatch05000029 =
{{
  0x13, 0x20, 0x21, 0x01, 0x29, 0x00, 0x00, 0x05,
  0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x10, 0x50, 0x00, 0x00, 0x00, 0xAA, 0xAA, 0xAA,
  0x0E, 0xAB, 0x99, 0xFE, 0xC0, 0x7C, 0x83, 0x1F,
  0xBB, 0x18, 0x59, 0x9E, 0xF8, 0xC1, 0xA6, 0x9E,
  0x64, 0xEC, 0x53, 0x29, 0x98, 0xD7, 0x25, 0x18,
  0xF1, 0x8D, 0x2D, 0xCB, 0x74, 0xED, 0x63, 0xD2,
  0xEF, 0xB0, 0xD5, 0x21, 0x31, 0x4D, 0x45, 0x1B,
  0xB5, 0x33, 0x72, 0xE6, 0x04, 0x32, 0x02, 0xBD,
  0x98, 0xD1, 0x99, 0x03, 0xFA, 0xE3, 0xE6, 0xBC,
  0xDF, 0x91, 0x65, 0x4D, 0xE0, 0xA0, 0x4E, 0x57,
  0x05, 0xA7, 0x91, 0x54, 0xD8, 0x96, 0x4C, 0x01,
  0x64, 0x49, 0x8A, 0x3C, 0x8A, 0xD8, 0x48, 0x04,
  0x9C, 0x9E, 0x33, 0xBE, 0xE4, 0xD8, 0x23, 0x05,
  0xDB, 0x42, 0x2B, 0xA8, 0x5A, 0x7B, 0x52, 0xF3,
  0xF4, 0x6A, 0xD4, 0x15, 0x5D, 0x03, 0x58, 0x98,
  0x10, 0x72, 0x73, 0x5B, 0x90, 0xBF, 0x40, 0x7B,
  0xB9, 0x80, 0xDC, 0xC8, 0x0C, 0xFB, 0xAE, 0xE3,
  0x52, 0xFA, 0x26, 0x08, 0x04, 0x13, 0x35, 0xC8,
  0x9D, 0x8B, 0x10, 0xBF, 0x60, 0x6B, 0x9B, 0xC0,
  0x97, 0xAF, 0x6F, 0x61, 0x1F, 0xB2, 0x62, 0xAF,
  0xC0, 0x06, 0x9A, 0xC5, 0x7E, 0xC1, 0xFF, 0xE9,
  0x16, 0x8D, 0xF2, 0x81, 0xA0, 0x6D, 0x39, 0x7E,
  0x74, 0x86, 0x4B, 0x5B, 0xA5, 0x48, 0xE7, 0x71,
  0x9F, 0x04, 0x84, 0xA1, 0xF2, 0x1E, 0x08, 0x5B,
  0x15, 0xAC, 0x1E, 0x32, 0xF8, 0xDC, 0xCD, 0x15,
  0x68, 0x3B, 0x06, 0xFA, 0x0A, 0x84, 0x99, 0xB1,
  0x8E, 0x49, 0x69, 0x83, 0xB0, 0x93, 0xC2, 0xA9,
  0x35, 0x4F, 0x80, 0x7F, 0x46, 0xC0, 0x6C, 0xA2,
  0xA2, 0xBB, 0xD4, 0x6F, 0x2C, 0x32, 0x54, 0xD2,
  0x91, 0xC7, 0x1E, 0x6D, 0xDF, 0xE2, 0xA2, 0xE1,
  0x69, 0x17, 0xC7, 0x8D, 0x85, 0xFB, 0x72, 0x3F,
  0xA8, 0x00, 0x08, 0x6A, 0xC2, 0xAC, 0x38, 0x9D,
  0x0B, 0xD1, 0xAE, 0x59, 0xFB, 0xE3, 0x00, 0xC1,
  0x39, 0x47, 0x73, 0x08, 0xB9, 0xA7, 0xD7, 0x1A,
  0x6C, 0x37, 0x07, 0x5F, 0xE8, 0x3F, 0x72, 0xAE,
  0x6D, 0x44, 0x2B, 0xE9, 0x10, 0x07, 0x97, 0x5D,
  0x2C, 0xFF, 0x16, 0x60, 0xF4, 0xE8, 0x80, 0x60,
  0x9F, 0x14, 0xF2, 0xED, 0x39, 0xE8, 0x60, 0xEC,
  0x24, 0xEE, 0xCE, 0xC0, 0xE5, 0x5A, 0x62, 0xCF,
  0x73, 0x1E, 0xDC, 0x20, 0x80, 0x7E, 0xAE, 0x3F,
  0xCF, 0x7C, 0x24, 0x7F, 0xCF, 0x45, 0x4F, 0x7F,
  0xFB, 0xB8, 0x1D, 0x37, 0x99, 0xCB, 0x5F, 0x79,
  0xC6, 0x1E, 0xF6, 0xB2, 0x83, 0x69, 0x57, 0x1D,
  0x81, 0xC7, 0xEA, 0x50, 0x58, 0x69, 0x26, 0xEC,
  0xE7, 0x31, 0x3F, 0x93, 0x90, 0x71, 0xB5, 0xB4,
  0x11, 0xC0, 0x19, 0x6E, 0x09, 0xA4, 0x01, 0x32,
  0x8D, 0xFE, 0x4A, 0x8C, 0x6C, 0xFB, 0xDD, 0xC2,
  0x06, 0x96, 0xD9, 0x5E, 0x0F, 0xD9, 0xB1, 0x6F,
  0x02, 0x0D, 0x8B, 0xEB, 0xCF, 0x94, 0x0B, 0xCA,
  0xEC, 0x21, 0x80, 0xFC, 0x83, 0x39, 0xA5, 0x18,
  0x27, 0xF3, 0x0D, 0x93, 0x58, 0xBD, 0x57, 0xE8,
  0x5D, 0xC7, 0x02, 0x4C, 0xAB, 0x79, 0x07, 0xDF,
  0x94, 0x8F, 0x1C, 0x99, 0x0F, 0x29, 0x96, 0x7F,
  0xDE, 0xFF, 0xC9, 0xC7, 0x15, 0xEC, 0x0E, 0x94,
  0x41, 0xF2, 0xA7, 0x2A, 0xB5, 0xE2, 0x1E, 0x41,
  0x05, 0x81, 0xDA, 0xFF, 0x0F, 0x1B, 0xCF, 0x61,
  0x75, 0xA8, 0x54, 0x62, 0xE4, 0xE9, 0x23, 0x9B,
  0x1A, 0xAE, 0x96, 0x37, 0x7C, 0x60, 0x18, 0xDE,
  0x76, 0x9A, 0x13, 0x8F, 0x0D, 0x67, 0xE7, 0x1B,
  0x5C, 0x14, 0xB8, 0x93, 0x64, 0x58, 0xE0, 0xCA,
  0xA8, 0x1D, 0x23, 0x67, 0x3A, 0x10, 0x1D, 0xA9,
  0xA2, 0x25, 0xA7, 0xFF, 0x5C, 0x91, 0x2D, 0xA4,
  0xD5, 0xE2, 0x65, 0x14, 0x58, 0xA8, 0x73, 0x76,
  0x26, 0xF0, 0xE5, 0xF8, 0xDB, 0xC6, 0xDA, 0x39,
  0xEE, 0xCC, 0x91, 0x57, 0x30, 0xC7, 0xE0, 0x9B,
  0xBA, 0x66, 0x53, 0x78, 0x07, 0xD2, 0xC1, 0x86,
  0x5B, 0xFD, 0xDA, 0x5F, 0xCF, 0x68, 0x62, 0x5D,
  0x18, 0x8B, 0xD2, 0x8A, 0x8D, 0x2A, 0x39, 0xA6,
  0x3D, 0x89, 0x8F, 0x65, 0x64, 0xE6, 0xD4, 0x5B,
  0x4D, 0x82, 0xDD, 0x80, 0xAA, 0x10, 0x5C, 0x0B,
  0x55, 0x68, 0x3D, 0x35, 0xDC, 0x0B, 0xF4, 0x18,
  0xBC, 0xC8, 0x48, 0x29, 0x08, 0x7A, 0x03, 0x14,
  0x32, 0xCB, 0x42, 0xCA, 0x6F, 0x1D, 0x3D, 0x53,
  0x3D, 0x7F, 0xC3, 0x96, 0x16, 0x3B, 0xB7, 0x40,
  0x98, 0xEE, 0xC6, 0x55, 0xC0, 0x02, 0x3E, 0xAE,
  0x0D, 0x99, 0x60, 0xC6, 0xF7, 0x0A, 0x43, 0xB4,
  0xF9, 0x1B, 0x5F, 0xEF, 0x6B, 0x5D, 0xED, 0x29,
  0xE1, 0x70, 0xF7, 0x92, 0x11, 0x01, 0xB6, 0x56,
  0xEF, 0xBD, 0x31, 0x55, 0xC1, 0xFC, 0xF8, 0xE4,
  0xFC, 0x37, 0x6E, 0xE6, 0x29, 0x25, 0xB3, 0x8D,
  0x50, 0x9B, 0x17, 0xE3, 0x24, 0x46, 0x2B, 0x70,
  0xF0, 0xD9, 0x2C, 0x70, 0x0C, 0xA0, 0x48, 0x4A,
  0x55, 0x3D, 0xB1, 0x2F, 0xEC, 0xC2, 0x0C, 0x36,
  0x3B, 0x36, 0x7F, 0x1D, 0xC3, 0x4C, 0xF5, 0xB7,
  0x88, 0x33, 0x40, 0x84, 0x99, 0x07, 0x84, 0x1E,
  0x20, 0x09, 0x98, 0x9F, 0x4A, 0x99, 0xFE, 0x88,
  0xB1, 0xE1, 0x7F, 0xED, 0xCF, 0x54, 0x4B, 0x3C,
  0xBF, 0x97, 0x46, 0x39, 0x0B, 0x8C, 0xCA, 0xA7,
  0xB5, 0x93, 0x5D, 0x65, 0xFF, 0x30, 0xFB, 0xDB,
  0x98, 0x94, 0x99, 0xE0, 0xE0, 0xD5, 0xFA, 0xF6,
  0xB3, 0xEC, 0x94, 0x5F, 0x78, 0x13, 0x5A, 0x3E,
  0x21, 0x2D, 0x76, 0xBC, 0xEF, 0xED, 0x94, 0x01,
  0xC5, 0x22, 0x6E, 0xD3, 0x77, 0x67, 0x81, 0xAD,
  0x63, 0xB4, 0x32, 0x8D, 0x50, 0x84, 0xCE, 0xF2,
  0xDD, 0xE4, 0xCF, 0xF7, 0xDF, 0x18, 0x9A, 0x03,
  0x24, 0x01, 0xA6, 0x9B, 0xF6, 0x5A, 0xD0, 0x15,
  0x9B, 0x80, 0x5A, 0x6E, 0x46, 0x1F, 0x60, 0x9F,
  0xE6, 0x71, 0xE3, 0x58, 0xFF, 0x8D, 0x6D, 0xC6,
  0x9D, 0x5E, 0x3A, 0x20, 0x08, 0x0C, 0x26, 0x18,
  0x51, 0xE2, 0xF8, 0x76, 0xC8, 0xD3, 0xDC, 0x41,
  0x82, 0x52, 0x7B, 0x69, 0x2A, 0x95, 0xCA, 0xD4,
  0x0A, 0x99, 0x2A, 0x73, 0xDD, 0xCB, 0xAC, 0x68,
  0xCF, 0xD6, 0x1B, 0x53, 0x0E, 0xA9, 0x08, 0x58,
  0x95, 0x7C, 0xF7, 0xEF, 0x45, 0x1F, 0x1B, 0x61,
  0x5D, 0x3D, 0xB7, 0xEB, 0x45, 0x74, 0x01, 0x0A,
  0xF2, 0x1D, 0x2C, 0xBF, 0x99, 0xF5, 0x9A, 0x5D,
  0x5E, 0xED, 0x4F, 0xC7, 0x79, 0x8B, 0x49, 0xEC,
  0x8D, 0x8B, 0xD3, 0xF0, 0x3C, 0xF5, 0xDA, 0x8C,
  0xAF, 0xAD, 0x72, 0xB7, 0x8D, 0x30, 0xDC, 0x9C,
  0x3C, 0x76, 0x17, 0x8A, 0xA7, 0x02, 0xF5, 0x01,
  0xBA, 0xBA, 0xF0, 0x71, 0x03, 0x6D, 0xEA, 0x13,
  0x8D, 0xF1, 0x6C, 0x3E, 0xE3, 0xB3, 0x0E, 0x1B,
  0x98, 0x60, 0x8A, 0xAD, 0xD8, 0x55, 0x25, 0x7F,
  0xE0, 0x00, 0x58, 0x21, 0x20, 0xA4, 0x2D, 0x83,
  0xAA, 0x39, 0x9A, 0x70, 0xBD, 0x9D, 0x4F, 0x6C,
  0xB2, 0x21, 0x2C, 0xB7, 0x75, 0xA5, 0xA5, 0xEC,
  0xB1, 0x5A, 0xFE, 0x91, 0x49, 0xD0, 0x90, 0x02,
  0xBB, 0x2E, 0xC4, 0xB4, 0x73, 0x58, 0x49, 0x53,
  0x31, 0xEB, 0x58, 0x66, 0xE2, 0xD2, 0x0D, 0xD2,
  0x9F, 0x7D, 0x9B, 0x4D, 0xA6, 0x72, 0x2A, 0xE4,
  0x04, 0xF0, 0x53, 0xF5, 0xDD, 0x8F, 0x8D, 0x0A,
  0xD0, 0xD3, 0xBA, 0xAA, 0x2F, 0x2E, 0xD1, 0x7A,
  0xED, 0xAD, 0x48, 0x21, 0xA7, 0x1E, 0xC9, 0x4F,
  0xF2, 0xE4, 0xAF, 0x62, 0x1D, 0x91, 0xB9, 0x9D,
  0xEA, 0x0F, 0x69, 0x23, 0x55, 0x99, 0xF0, 0xD2,
  0x92, 0xD8, 0x20, 0xC0, 0xBC, 0x75, 0xA7, 0x83,
  0x5B, 0x6F, 0x4A, 0x2F, 0xA8, 0x35, 0x2B, 0xC3,
  0xBA, 0xFB, 0x9B, 0xB9, 0x11, 0xF7, 0x8E, 0xF5,
  0x0E, 0x9B, 0x51, 0xB7, 0xFB, 0x59, 0x63, 0x17,
  0x37, 0x5A, 0x7E, 0x20, 0xAF, 0x97, 0xAD, 0x61,
  0xA0, 0xFA, 0xFC, 0x95, 0x56, 0xD9, 0x08, 0x34,
  0x76, 0xEF, 0xE9, 0xA1, 0x06, 0xB5, 0xEA, 0xC5,
  0xDE, 0x7F, 0xE6, 0xBC, 0x9E, 0x2A, 0x51, 0x4D,
  0x9C, 0x42, 0x96, 0x3F, 0xAF, 0xA5, 0x49, 0x78,
  0x9E, 0xF1, 0xFD, 0x2A, 0xD4, 0x3D, 0xA7, 0x6B,
  0x9F, 0xDF, 0x37, 0x7A, 0xAB, 0x99, 0x00, 0xE2,
  0xA1, 0x9C, 0xDB, 0x54, 0x8F, 0xF2, 0xBB, 0x1B,
  0x7F, 0x28, 0x84, 0x20, 0x65, 0xD5, 0x2A, 0xA6,
  0x0D, 0x62, 0x4F, 0x2D, 0xF0, 0xAF, 0x40, 0xFD,
  0x2E, 0xB1, 0x72, 0xCD, 0x1D, 0x9A, 0xC4, 0x71,
  0xFD, 0x4B, 0xB6, 0x5B, 0x13, 0xDB, 0x78, 0xDD,
  0x79, 0xC6, 0x05, 0x22, 0x14, 0x98, 0xD5, 0x10,
  0xC5, 0x5A, 0xBE, 0x11, 0xD6, 0xC1, 0x4B, 0x39,
  0xD1, 0x88, 0x80, 0x79, 0x86, 0xDD, 0x1F, 0x54,
  0x87, 0x96, 0x3E, 0xDE, 0x43, 0x25, 0xB2, 0xA6,
  0x6E, 0xD0, 0xA4, 0x69, 0x67, 0x33, 0xE1, 0xD9,
  0x24, 0xFB, 0x67, 0xD0, 0xC9, 0x4F, 0xB4, 0x3E,
  0x48, 0x66, 0x42, 0x64, 0x42, 0xB6, 0x49, 0x99,
  0x0F, 0x4D, 0xCD, 0xC8, 0x1B, 0x0E, 0x1B, 0x47,
  0x8C, 0x57, 0x2A, 0x02, 0xBA, 0x43, 0x53, 0x74,
  0xD8, 0x74, 0x28, 0x05, 0x03, 0x16, 0x1D, 0x73,
  0x2F, 0x22, 0x0B, 0xA4, 0xFB, 0xBC, 0xE3, 0x11,
  0xCA, 0xAF, 0x90, 0x1F, 0xA8, 0x67, 0xE1, 0xA1,
  0x28, 0x68, 0xA5, 0x9D, 0x32, 0x85, 0xB3, 0x53,
  0x5E, 0x7A, 0xF4, 0xE8, 0xB1, 0x96, 0xCF, 0xB8,
  0xAA, 0x18, 0x7C, 0xEB, 0xF9, 0x33, 0x3E, 0x95,
  0xF6, 0xFC, 0x9A, 0x5A, 0xAC, 0x60, 0x9F, 0xC9,
  0xB8, 0x6D, 0xA2, 0x77, 0xFA, 0x64, 0xBA, 0x7B,
  0xB5, 0x4D, 0x60, 0x49, 0xE3, 0x28, 0x04, 0x7C,
  0xAD, 0xA1, 0xA3, 0xB6, 0x54, 0xB4, 0xAE, 0xAE,
  0x0E, 0xA7, 0x83, 0x48, 0x64, 0xB0, 0x52, 0x5B,
  0x12, 0xBE, 0x36, 0x72, 0x8C, 0xAC, 0x5D, 0x30,
  0x5C, 0xC2, 0x0F, 0x5A, 0x92, 0x95, 0x34, 0xC4,
  0x66, 0xFF, 0x5B, 0x32, 0x97, 0x07, 0xE7, 0x06,
  0x5B, 0x10, 0x1D, 0xF9, 0xA7, 0xDA, 0x87, 0xFD,
  0x76, 0x79, 0xF5, 0xD1, 0xE7, 0xE8, 0xA0, 0x03,
  0x59, 0x70, 0xE4, 0x1A, 0x34, 0x73, 0xAA, 0xB0,
  0x6A, 0x81, 0xC8, 0x24, 0x32, 0x48, 0x9E, 0x13,
  0x3A, 0xFB, 0x03, 0xF3, 0xFF, 0x72, 0x4F, 0xD5,
  0xC6, 0xE5, 0xDD, 0xCA, 0x6F, 0x37, 0xB4, 0x67,
  0x77, 0x40, 0x8F, 0x2F, 0xFE, 0xF0, 0x61, 0xE0,
  0x94, 0x45, 0x0E, 0x53, 0x9A, 0xC5, 0xC1, 0x8E,
  0xA1, 0xA6, 0x73, 0xD3, 0xF4, 0x73, 0x18, 0xDC,
  0xB1, 0xF3, 0xB7, 0x9B, 0xF5, 0xFF, 0x50, 0x61,
  0x27, 0xD5, 0x72, 0xED, 0x64, 0x9D, 0x08, 0x84,
  0x5F, 0x6E, 0x7F, 0x6E, 0xDF, 0x77, 0x0C, 0xC0,
  0x19, 0xDB, 0x77, 0x54, 0x00, 0x68, 0xF8, 0xA8,
  0x66, 0xF8, 0x3E, 0xE6, 0x26, 0x32, 0x32, 0xE9,
  0xB0, 0xB7, 0x74, 0xFD, 0xD8, 0x58, 0x58, 0x69,
  0xB0, 0x1E, 0xD3, 0x65, 0x70, 0x23, 0x79, 0x9E,
  0xD9, 0x34, 0x84, 0xEB, 0xB9, 0xE4, 0x49, 0xB9,
  0xF4, 0x31, 0x05, 0x31, 0x27, 0xF0, 0x80, 0x1F,
  0xBE, 0xD5, 0x5F, 0x1F, 0x17, 0x9D, 0x0C, 0x7B,
  0x94, 0x3E, 0x63, 0x4C, 0xF9, 0x2F, 0x76, 0xB3,
  0xC1, 0xEF, 0x34, 0x20, 0xA4, 0x4B, 0x5A, 0x29,
  0xAA, 0x40, 0xA9, 0x0C, 0xF2, 0x24, 0x27, 0xE8,
  0xF1, 0xBF, 0xDA, 0x79, 0x75, 0x75, 0xE6, 0xA1,
  0xAB, 0xD3, 0xB5, 0xCD, 0xEA, 0x1E, 0x6A, 0x9D,
  0x74, 0x15, 0x77, 0x8D, 0x7A, 0x5B, 0xDA, 0x2F,
  0x76, 0x7E, 0xE9, 0xFE, 0xEA, 0x88, 0x22, 0x35,
  0x94, 0x60, 0x31, 0x3D, 0xA9, 0x83, 0x31, 0xBB,
  0x3E, 0xEE, 0x97, 0x21, 0x0B, 0x1D, 0x76, 0x6D,
  0x00, 0x9A, 0x45, 0x60, 0x7E, 0x79, 0xED, 0x70,
  0x8C, 0xB1, 0xA2, 0xA1, 0xC0, 0x63, 0x37, 0x5D
}};


/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef PORTING_H
#define PORTING_H


/* For AMD64 or 32-bit GCC */
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef char int8;
typedef unsigned char  uint8;

/* Create the Boolean type */
#define TRUE  1
#define FALSE 0
typedef unsigned char BOOL;

#define CALLCONV


typedef struct _uint64
{
	uint32 lo;
	uint32 hi;
}uint64;


/*
 *   SBDFO - Segment Bus Device Function Offset
 *   31:28   Segment (4-bits)
 *   27:20   Bus     (8-bits)
 *   19:15   Device  (5-bits)
 *   14:12   Function(3-bits)
 *   11:00   Offset  (12-bits)
 */
typedef uint32 SBDFO;

#define MAKE_SBDFO(seg,bus,dev,fun,off) ((((uint32)(seg))<<28) | (((uint32)(bus))<<20) | \
		    (((uint32)(dev))<<15) | (((uint32)(fun))<<12) | ((uint32)(off)))
#define SBDFO_SEG(x) (((uint32)(x)>>28) & 0x0F)
#define SBDFO_BUS(x) (((uint32)(x)>>20) & 0xFF)
#define SBDFO_DEV(x) (((uint32)(x)>>15) & 0x1F)
#define SBDFO_FUN(x) (((uint32)(x)>>12) & 0x07)
#define SBDFO_OFF(x) (((uint32)(x)) & 0xFFF)
#define ILLEGAL_SBDFO 0xFFFFFFFF

void CALLCONV AmdMSRRead(uint32 Address, uint64 *Value);
void CALLCONV AmdMSRWrite(uint32 Address, uint64 *Value);
void CALLCONV AmdIORead(uint8 IOSize, uint16 Address, uint32 *Value);
void CALLCONV AmdIOWrite(uint8 IOSize, uint16 Address, uint32 *Value);
void CALLCONV AmdMemRead(uint8 MemSize, uint64 *Address, uint32 *Value);
void CALLCONV AmdMemWrite(uint8 MemSize, uint64 *Address, uint32 *Value);
void CALLCONV AmdPCIRead(SBDFO loc, uint32 *Value);
void CALLCONV AmdPCIWrite(SBDFO loc, uint32 *Value);
void CALLCONV AmdCPUIDRead(uint32 Address, uint32 Regs[4]);

#define BYTESIZE 1
#define WORDSIZE 2
#define DWORDSIZE 4

#endif /* PORTING_H */

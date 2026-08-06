#ifndef SLUS_004_22_SYMBOLS_VARIABLES_H
#define SLUS_004_22_SYMBOLS_VARIABLES_H

#include "bof3/bof3.h"

/* Core runtime state near the SLUS callback scheduler. */
/* @source 0x80143D44 @kind unknown */
extern u8  D_80143D44;
/* @source 0x80143D75 @kind unknown */
extern u8  D_80143D75[];
/* @source 0x80143E05 @kind unknown */
extern u8  D_80143E05[];
/* @source 0x80143D48 @kind unknown */
extern u8  D_80143D48[];
/* @source 0x80143E68 @kind unknown */
extern u8* D_80143E68;
/* @source 0x80143E6C @kind unknown */
extern s32 D_80143E6C;
/* @source 0x80143EF8 @kind unknown */
extern s32 D_80143EF8;
/* @source 0x80143EFC @kind unknown */
extern s32 D_80143EFC;
/* @source 0x80143F44 @kind unknown */
extern u8  D_80143F44;

/* Shared SLUS data referenced by reviewed core functions. */
/* @source 0x80145AA4 @kind unknown */
extern u16 D_80145AA4;

/* Static target data. */
/* @source 0x8018B300 @kind unknown */
extern u8 D_8018B300;

/* Function-local data for dequeueIntRpIrq. */
/* @source 0x8018DB40 @kind unknown */
extern u32 D_8018DB40[];

#endif

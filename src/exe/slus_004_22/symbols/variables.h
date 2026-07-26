#ifndef SLUS_004_22_SYMBOLS_VARIABLES_H
#define SLUS_004_22_SYMBOLS_VARIABLES_H

#include "bof3/bof3.h"

/* Core runtime state near the SLUS callback scheduler. */
extern u8  D_80143D44;
extern u8  D_80143D48[];
extern u8* D_80143E68;
extern s32 D_80143E6C;
extern s32 D_80143EF8;
extern s32 D_80143EFC;
extern u8  D_80143F44;

/* Shared SLUS data referenced by reviewed core functions. */
extern u16 D_80145AA4;

/* Static target data. */
extern u8 D_8018B300;

/* Function-local data for func_801729D0. */
extern u16 D_8018E7EE;
extern u8  D_8018E264;
extern u32 D_8018E258;
extern u32 D_8018E250;
extern u32 D_8018E25C;
extern u8  D_8018E0E8[];

#endif

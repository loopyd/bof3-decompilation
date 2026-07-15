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
extern u8  D_8014B17C;

/* Static target data. */
extern u8 D_8018B300;

#endif

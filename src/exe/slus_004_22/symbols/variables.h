#ifndef SLUS_004_22_SYMBOLS_VARIABLES_H
#define SLUS_004_22_SYMBOLS_VARIABLES_H

#include "bof3/bof3.h"

/* Core runtime state near the SLUS callback scheduler. */
extern u8  DAT_80143d44;
extern u8  DAT_80143d48[];
extern u8* DAT_80143e68;
extern s32 DAT_80143e6c;
extern s32 DAT_80143ef8;
extern s32 DAT_80143efc;
extern u8  DAT_80143f44;

/* Shared SLUS data referenced by reviewed core functions. */
extern u16 DAT_80145aa4;
extern u8  DAT_8014b17c;

/* Static target data. */
extern u8 DAT_8018b300;

#endif

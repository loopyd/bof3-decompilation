#ifndef EMI_SCENA00_00_INTERNAL_H
#define EMI_SCENA00_00_INTERNAL_H

#include "bof3/bof3.h"

typedef void (*Scena00RecordCallback)(void* record, u32 arg1);

/* @source 0x8014686C @kind unknown */
extern volatile u32        D_8014686C;
extern Scena00RecordCallback D_801FCA84[];

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);

void func_801F7134(s32 chapter_id);
void func_801F78EC(s32 x0, s32 y0, s16 angle0, s32 x1, s32 y1, s16 angle1, u8 r,
                   u8 g, u8 b);
s32  returnZero(void);
void resetEffectBank(void);

#endif

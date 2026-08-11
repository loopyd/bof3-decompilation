#ifndef EMI_SCE10EFF_00_INTERNAL_H
#define EMI_SCE10EFF_00_INTERNAL_H

#include "bof3/context.h"
#include "memory/scratchpad.h"

typedef struct ScenarioSce10effScratch {
  u8  pad_00[0x08];
  u8  flags_08;
  u8  pad_09;
  u8  color_0a;
  u8  pad_0b[0x23];
  u16 screen_x_2e;
  u16 screen_y_30;
  u8  pad_32[0x02];
  s32 unk_34;
  s32 unk_38;
  u8  pad_3c[0x02];
  u16 unk_3e;
} ScenarioSce10effScratch;

/* @source 0x1F800044 @kind unknown */
extern ScenarioSce10effScratch* D_1F800044;

/* @source 0x801D2708 @kind table */
extern void (*D_801D2708[])(void);

void func_8017AA30(void* arg0);
void func_8017A97C(void* arg0);
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);
void func_801D2658(void);

#endif

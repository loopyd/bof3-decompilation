#ifndef BOF3_SRC_MODULES_WORLD00_AREA008_13_INTERNAL_H
#define BOF3_SRC_MODULES_WORLD00_AREA008_13_INTERNAL_H

#include "bof3/bof3.h"

typedef struct World00Area008Scratch {
  u8 unk_00[0x5d];
  s8 field_5d;
  s8 field_5e;
} World00Area008Scratch;

#define WORLD00_AREA008_SCRATCH_PTR \
  VPPTR(World00Area008Scratch, 0x1f800044u)
#define WORLD00_AREA008_PRIMITIVE_PTR    VPPTR(u8, 0x8014598cu)
#define WORLD00_AREA008_UI_CHAR_BUFFER   VPTR(u8, 0x80145ad4u)
extern vu8 WORLD00_AREA008_GLOBAL_BYTE_6867;
void func_8017e3f4(void* arg0, const void* arg1, ...);
void func_8014ff0c(s16 arg0, s16 arg1, s32 arg2, const void* arg3);
void func_8014f800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);
void func_8014e5a0(u8 arg0, u8 arg1);
void func_801aeba0(s16 arg0, s16 arg1, s16 arg2, s16 arg3, s32 arg4);

/* from old bof3/include/bof3/modules/world00/area008/13.h */
void func_801f3c2c(void);
void func_801f3d18(void);
void func_801f3d88(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4);

#endif

#ifndef EMI_WORLD00_AREA008_13_INTERNAL_H
#define EMI_WORLD00_AREA008_13_INTERNAL_H

#include "bof3/bof3.h"

typedef struct World00Area008Scratch {
  u8 unk_00[0x5d];
  s8 field_5d;
  s8 field_5e;
} World00Area008Scratch;

#define WORLD00_AREA008_SCRATCH_PTR \
  PTR_SLOT_AT(volatile World00Area008Scratch, 0x1f800044u)
#define WORLD00_AREA008_PRIMITIVE_PTR PTR_SLOT_AT(volatile u8, 0x8014598cu)
extern vu8  WORLD00_AREA008_D_80146867;
extern vu16 WORLD00_AREA008_D_8014932A;
extern vu8  WORLD00_AREA008_D_80149333;
extern u8   WORLD00_AREA008_D_80145AD4[];
extern u8   WORLD00_AREA008_D_801F2C04[];
extern u8   WORLD00_AREA008_D_801F2C10[];
s32         func_8017E3F4(char* buffer, const char* format, ...);
void        func_8014FF0C(s16 arg0, s16 arg1, s32 arg2, const void* arg3);
void        func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);
void        func_8014E5A0(u8 arg0, u8 arg1);
void        func_801AEBA0(s16 arg0, s16 arg1, s16 arg2, s16 arg3, s32 arg4);

void func_801F3C2C(void);
void func_801F3D18(void);
void func_801F3D88(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4);

#endif

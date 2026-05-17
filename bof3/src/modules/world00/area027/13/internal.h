#ifndef BOF3_SRC_MODULES_WORLD00_AREA027_13_INTERNAL_H
#define BOF3_SRC_MODULES_WORLD00_AREA027_13_INTERNAL_H

#include "bof3/modules/world00/area027/13.h"
#include "bof3/context.h"

#define WORLD00_AREA027_PRIMITIVE_PTR (*(volatile u8**)0x8014598cu)
#define WORLD00_AREA027_SCRATCH_PTR   (*(volatile u8**)0x1f800044u)
#define WORLD00_AREA027_MATRIX_92E8   ((MATRIX*)0x801492e8u)

typedef struct World00Area027Point {
  s16 x;
  s16 y;
  s16 z;
} World00Area027Point;

void func_801afe18(void* arg0);
void func_801aff04(const void* arg0, void* arg1);
void func_80155a08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
s32  func_80155560(u32 arg0, void* arg1, s32 arg2);
s32  func_8015b5d4(u32 arg0, s32 arg1);
u16  func_8017a620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8017a904(void* arg0, s32 arg1);
void func_8017aa94(void* arg0);
void func_8017c2d8(void* arg0, s32 arg1, s32 arg2, s32 arg3, void* arg4);
void func_8014e5a0(u8 arg0, u8 arg1);

#endif

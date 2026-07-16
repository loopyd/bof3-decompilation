#ifndef EMI_WORLD00_AREA027_13_INTERNAL_H
#define EMI_WORLD00_AREA027_13_INTERNAL_H

#include "bof3/bof3.h"

#include "symbols/symbols.h"

#define WORLD00_AREA027_PRIMITIVE_PTR VPPTR(u8, 0x8014598cu)
#define WORLD00_AREA027_SCRATCH_PTR   VPPTR(u8, 0x1f800044u)
#define WORLD00_AREA027_MATRIX_92E8   ((MATRIX*)0x801492e8u)

typedef struct World00Area027Point {
  s16 x;
  s16 y;
  s16 z;
} World00Area027Point;

void func_801AFE18(void* arg0);
void func_801AFF04(const void* arg0, void* arg1);
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
s32  func_80155560(u32 arg0, void* arg1, s32 arg2);
s32  func_8015B5D4(u32 arg0, s32 arg1);
u16  func_8017A620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8017A904(void* arg0, s32 arg1);
void func_8017AA94(void* arg0);
void func_8017C2D8(void* arg0, s32 arg1, s32 arg2, s32 arg3, void* arg4);
void func_8014E5A0(u8 arg0, u8 arg1);

void func_801F2E3C(void* arg0);
void func_801F2F0C(const void* arg0);
void func_801F304C(void* arg0);
void func_801F31CC(void);
void func_801F33A8(void);
void func_801F3480(const void* arg0, s32 arg1, u32 arg2);

#endif

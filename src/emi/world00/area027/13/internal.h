#ifndef EMI_WORLD00_AREA027_13_INTERNAL_H
#define EMI_WORLD00_AREA027_13_INTERNAL_H

#include "bof3/bof3.h"

#include "symbols/symbols.h"

typedef struct World00Area027Point {
  s16 x;
  s16 y;
  s16 z;
} World00Area027Point;

// @source 0x80144E98
// @kind unknown
extern u8 D_80144E98[];
// @source 0x1F800044
// @kind unknown
extern u8* D_1F800044;

void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
s32  func_80155560(u32 arg0, void* arg1, s32 arg2);
s32  func_8015B5D4(u32 arg0, s32 arg1);
void func_8017AA94(void* arg0);
void func_8014E5A0(u8 arg0, u8 arg1);

void func_801F2E3C(void* arg0);
void emitTrailStrip(const void* arg0);
void func_801F304C(void* arg0);
void emitMarkerPair(void);
void func_801F33A8(void);
void func_801F3480(const void* arg0, s32 arg1, u32 arg2);

#define WORLD00_AREA027_PRIMITIVE_PTR PSX_REF(volatile u8*, 0x8014598cu)
#define WORLD00_AREA027_SCRATCH_PTR   D_1F800044
#define WORLD00_AREA027_MATRIX_92E8   PSX_PTR(MATRIX, 0x801492e8u)

#endif

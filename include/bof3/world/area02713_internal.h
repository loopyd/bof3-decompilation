#ifndef EMI_WORLD00_AREA027_13_INTERNAL_H
#define EMI_WORLD00_AREA027_13_INTERNAL_H

#include "bof3/bof3.h"

typedef struct World00Area027Point {
  s16 x;
  s16 y;
  s16 z;
} World00Area027Point;

/* @source 0x80144E98 @kind unknown */
extern u8 D_80144E98[];
/* @source 0x1F800044 @kind unknown */
extern u8* D_1F800044;
/* @source 0x801492E8 @kind unknown */
extern MATRIX D_801492E8;
/* @source 0x1F800014 @kind unknown */
extern SVECTOR D_1F800014[];
/* @source 0x80146864 @kind unknown */
extern volatile u8 g_ScenarioProgress;
/* @source 0x801490C7 @kind unknown */
extern s8 D_801490C7;
/* @source 0x801F3AB4 @kind table */
extern void (*handlerTable[])(void);
/* @source 0x801F3ABC @kind table */
extern void (*D_801F3ABC[])(void);

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
POLY_FT4* emitMarkerQuad(const void* arg0, s32 arg1, u32 arg2);
void func_801F3618(void);
void func_801F3650(void);
void func_801F36D0(void);

#define WORLD00_AREA027_PRIMITIVE_PTR PSX_REF(volatile u8*, 0x8014598cu)
#define WORLD00_AREA027_SCRATCH_PTR   D_1F800044
#define WORLD00_AREA027_MATRIX_92E8   (&D_801492E8)
#define WORLD00_AREA027_FLAG_48EB     PSX_REF(u8, 0x801448ebu)
#define WORLD00_AREA027_FLAG_48EC     PSX_REF(u8, 0x801448ecu)
#define WORLD00_AREA027_STATE_90A8    PSX_REF(u16, 0x801490a8u)
#define WORLD00_AREA027_FLAGS_90C7    PSX_REF(s8, 0x801490c7u)

#endif

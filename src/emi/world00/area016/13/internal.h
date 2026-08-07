#ifndef EMI_WORLD00_AREA016_13_INTERNAL_H
#define EMI_WORLD00_AREA016_13_INTERNAL_H

#include "bof3/bof3.h"

#include "symbols/symbols.h"

typedef void (*World00Area016Handler)(void);

typedef struct World00Area016MarkerEntry {
  u16 mask;
  u8  field_02;
  u8  field_03;
} World00Area016MarkerEntry;

typedef struct World00Area016Scratch {
  u8  unk_00;
  u8  mode;
  u8  state_02;
  u8  state_03;
  u8  unk_04[0x2a];
  s16 field_2e;
  s16 field_30;
} World00Area016Scratch;

/* @source 0x801F5114 @kind unknown */
extern World00Area016Handler WORLD00_AREA016_D_801F5114[];
/* @source 0x801F511C @kind unknown */
extern World00Area016Handler WORLD00_AREA016_D_801F511C[];
/* @source 0x801F512C @kind unknown */
extern World00Area016Handler WORLD00_AREA016_D_801F512C[];
/* @source 0x801F51AC @kind unknown */
extern World00Area016Handler WORLD00_AREA016_D_801F51AC[];

void func_8014E5A0(u8 arg0, u8 arg1);
void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);
s8   func_80166CB0(s16 arg0, s16 arg1);
u8   func_801B6610(s16 arg0, s16 arg1);

void seedScratchDefaults(void);
void dispatchState02(void);
void advanceState02Step(void);
void dispatchState03(void);
void func_801F39D8(s16 arg0, s16 arg1, u32 arg2);
void func_801F3B00(s32 arg0, s32 arg1);
void func_801F3ECC(s16 arg0, s16 arg1);
void func_801F40C4(s16 arg0, s16 arg1);
void func_801F4178(void);

#define WORLD00_AREA016_SCRATCH_PTR                                            \
  PSX_REF(volatile World00Area016Scratch*, 0x1f800044u)
#define WORLD00_AREA016_PRIMITIVE_PTR PSX_REF(volatile u8*, 0x8014598cu)
#define WORLD00_AREA016_SPRT_TABLE PSX_PTR(const volatile u8, 0x801f513cu)
#define WORLD00_AREA016_MARKER_TABLE                                           \
  PSX_PTR(const volatile World00Area016MarkerEntry, 0x801f5194u)
#define WORLD00_AREA016_ROTATION   PSX_PTR(SVECTOR, 0x801492d8u)
#define WORLD00_AREA016_G4_VERTEX0 SPAD_ADDR(SVECTOR, 0x14u)
#define WORLD00_AREA016_G4_VERTEX1 SPAD_ADDR(SVECTOR, 0x1cu)
#define WORLD00_AREA016_G4_VERTEX2 SPAD_ADDR(SVECTOR, 0x24u)
#define WORLD00_AREA016_G4_VERTEX3 SPAD_ADDR(SVECTOR, 0x2cu)

#endif

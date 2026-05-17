#ifndef BOF3_SRC_MODULES_WORLD00_AREA016_13_INTERNAL_H
#define BOF3_SRC_MODULES_WORLD00_AREA016_13_INTERNAL_H

#include "bof3/bof3.h"

typedef void (*World00Area016Handler)(void);

typedef struct World00Area016MarkerEntry {
  u16 mask;
  u8  field_02;
  u8  field_03;
} World00Area016MarkerEntry;

typedef struct World00Area016Scratch {
  u8  unk_00[2];
  u8  state_02;
  u8  state_03;
  u8  unk_04[0x2a];
  s16 field_2e;
  s16 field_30;
} World00Area016Scratch;

#define WORLD00_AREA016_SCRATCH_PTR \
  VPPTR(World00Area016Scratch, 0x1f800044u)
#define WORLD00_AREA016_PRIMITIVE_PTR    VPPTR(u8, 0x8014598cu)
#define WORLD00_AREA016_GLOBAL_BYTE_54F2 VU8(0x801454f2u)
#define WORLD00_AREA016_GLOBAL_HALF_5AB4 VU16(0x80145ab4u)
#define WORLD00_AREA016_GLOBAL_HALF_5AC0 VU16(0x80145ac0u)
#define WORLD00_AREA016_GLOBAL_HALF_6258 VU16(0x80146258u)
#define WORLD00_AREA016_GLOBAL_HALF_625A VU16(0x8014625au)
#define WORLD00_AREA016_GLOBAL_BYTE_832E VU8(0x8014832eu)
#define WORLD00_AREA016_GLOBAL_HALF_930A VS16(0x8014930au)
#define WORLD00_AREA016_GLOBAL_HALF_930E VS16(0x8014930eu)
#define WORLD00_AREA016_STREAM_HINT      VU8(0x80145024u)
#define WORLD00_AREA016_STATE_TABLE \
  ((World00Area016Handler const volatile*)0x801f511cu)
#define WORLD00_AREA016_STATE_TABLE_03 \
  ((World00Area016Handler const volatile*)0x801f512cu)
#define WORLD00_AREA016_SPRT_TABLE ((const volatile u8*)0x801f513cu)
#define WORLD00_AREA016_MARKER_TABLE \
  ((const volatile World00Area016MarkerEntry*)0x801f5194u)
#define WORLD00_AREA016_ROTATION       ((SVECTOR*)0x801492d8u)
#define WORLD00_AREA016_G4_VERTEX0     ((SVECTOR*)0x1f800014u)
#define WORLD00_AREA016_G4_VERTEX1     ((SVECTOR*)0x1f80001cu)
#define WORLD00_AREA016_G4_VERTEX2     ((SVECTOR*)0x1f800024u)
#define WORLD00_AREA016_G4_VERTEX3     ((SVECTOR*)0x1f80002cu)
#define WORLD00_AREA016_BOOT_HALF_0008 VU16(0x80010008u)

void func_8014e5a0(u8 arg0, u8 arg1);
void func_8014f800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);
s8   func_80166cb0(s16 arg0, s16 arg1);
u8   func_801b6610(s16 arg0, s16 arg1);

/* from old bof3/include/bof3/modules/world00/area016/13.h */
void func_801f3400(void);
void func_801f34c8(void);
void func_801f35b8(void);
void func_801f368c(void);
void func_801f39d8(s16 arg0, s16 arg1, u32 arg2);
void func_801f3b00(s32 arg0, s32 arg1);
void func_801f3ecc(s16 arg0, s16 arg1);
void func_801f40c4(s16 arg0, s16 arg1);

#endif

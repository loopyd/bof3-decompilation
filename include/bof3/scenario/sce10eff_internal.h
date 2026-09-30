#ifndef EMI_SCE10EFF_00_INTERNAL_H
#define EMI_SCE10EFF_00_INTERNAL_H

#include "bof3/context.h"
#include "memory/scratchpad.h"

typedef struct ScenarioSce10effScratch {
  u8  pad_00[0x08];
  u8  flags_08;
  u8  pad_09;
  u8  color_0a;
  u8  pad_0b;
  s32 unk_0c;
  u8  pad_10[0x1e];
  u16 screen_x_2e;
  u16 screen_y_30;
  u8  pad_32[0x02];
  s32 unk_34;
  s32 unk_38;
  u8  pad_3c[0x02];
  u16 unk_3e;
} ScenarioSce10effScratch;

/* Front-end entry table at 0x80143FC8 — 0x74 bytes per record. Only the
 * occupancy byte at 0x00 and the slot bytes at 0x01-0x05 are proven for this
 * target. */
typedef struct ScenarioSce10effRecord {
  u8 flags_00;
  u8 unk_01;
  u8 unk_02;
  u8 unk_03;
  u8 unk_04;
  u8 unk_05;
  u8 pad_06[0x6E]; /* 0x06 - 0x73 */
} ScenarioSce10effRecord;

/* @source 0x1F800000 @kind unknown */
extern volatile s32 D_1F800000;

/* @source 0x1F800004 @kind unknown */
extern volatile s32 D_1F800004;

/* @source 0x1F800008 @kind unknown */
extern volatile s32 D_1F800008;

/* @source 0x1F80000C @kind unknown */
extern volatile s32 D_1F80000C;

/* @source 0x1F800014 @kind unknown */
extern SVECTOR D_1F800014;

/* Halfwords of the scratch quad-band records at 0x1F800014 that the band
 * builder loads and rewrites between projections. */

/* @source 0x1F800016 @kind unknown */
extern u16 D_1F800016;

/* @source 0x1F80001E @kind unknown */
extern u16 D_1F80001E;

/* @source 0x1F800026 @kind unknown */
extern u16 D_1F800026;

/* @source 0x1F80002E @kind unknown */
extern u16 D_1F80002E;

/* @source 0x1F800044 @kind unknown */
extern ScenarioSce10effScratch* D_1F800044;

/* @source 0x80143FC8 @kind unknown */
extern ScenarioSce10effRecord D_80143FC8[];

/* @source 0x801D2708 @kind table */
extern void (*D_801D2708[])(void);

/* @source 0x801D2714 @kind table */
extern void (*D_801D2714[])(void);

/* @source 0x801D2724 @kind table */
extern void (*D_801D2724[])(void);

/* @source 0x801D2734 @kind table */
extern void (*D_801D2734[])(void);

/* @source 0x801D2744 @kind unknown */
extern u8* D_801D2744;

s16 func_8015477C(s32 arg0, s32 arg1);
void func_801D0C70(void);
void func_801D0E64(void);
void func_801D0E9C(void);
void func_8015DF18(u16 arg0);
void func_8017AA30(void* arg0);
void func_8017A97C(void* arg0);
void func_8017AA94(void* arg0);
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);
u8 func_8019601C(void);
void func_80196070(void);
void func_801D165C(s32 arg0, s16 arg1, s16 arg2);
void func_801D218C(void);
void func_801D2658(void);
void setupSceneObjectTransform(void);
void emitRadialTranslucentQuads(void);

#endif

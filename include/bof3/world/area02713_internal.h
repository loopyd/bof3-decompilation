#ifndef EMI_WORLD00_AREA027_13_INTERNAL_H
#define EMI_WORLD00_AREA027_13_INTERNAL_H

#include "bof3/bof3.h"

typedef struct World00Area027Point {
  s16 x;
  s16 y;
  s16 z;
} World00Area027Point;

/* One 4-byte projected screen pair of the local trail: two signed halfwords,
 * so the original copies whole entries with the unaligned lwl/lwr + swl/swr
 * idiom. */
typedef struct World00Area027ScreenPoint {
  s16 x;
  s16 y;
} World00Area027ScreenPoint;

/* Entry table at 0x80143FC8 — 0x74 bytes per record. Only the occupancy byte
 * at 0x00 and the byte at 0x05 are proven for this target. */
typedef struct World00Area027Record {
  u8 flags_00;
  u8 unk_01;
  u8 unk_02;
  u8 unk_03;
  u8 unk_04;
  u8 unk_05;
  u8 pad_06[0x6E]; /* 0x74 - 6 */
} World00Area027Record;

/* Work records at 0x800E4800 — 0x98 bytes per record. Only the words at
 * 0x00/0x04/0x08/0x10, the halfword at 0x14 and the 32-entry screen-pair trail
 * at 0x18 are proven for this target; the local initializer at 0x801F304C
 * consumes the words as the fixed-point inputs and the 12-bit turn angle of its
 * position/rotation pair, and the local trail projector at 0x801F2E3C fills the
 * trail with the record's projected rotated centre. */
typedef struct World00Area027Work {
  u32 unk_00;
  u32 unk_04;
  u32 unk_08;
  u32 unk_0c;
  u32 unk_10; /* radius; the trail projector multiplies it as a signed value */
  u16 unk_14;
  u8  pad_16[2]; /* 0x18 - 0x16 */
  World00Area027ScreenPoint trail_18[0x20];
} World00Area027Work;

/* @source 0x80143FC8 @kind bss */
extern World00Area027Record D_80143FC8[];
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
/* @source 0x801490A8 @kind unknown */
extern u16 D_801490A8;
/* @source 0x801490C7 @kind unknown */
extern s8 D_801490C7;
/* @source 0x801448EB @kind unknown — shared front-end flag cleared by this
 * area's startup state machine. */
extern u8 D_801448EB;
/* @source 0x801448EC @kind unknown — shared front-end mode byte driven by this
 * area's startup state machine through the values 1..4; the original reads it
 * through a signed byte load and dispatches it through a five-entry jump
 * table, so the target-local view is signed. */
extern s8 D_801448EC;
/* @source 0x801448EE @kind unknown — shared halfword seeded with 0x10 and then
 * counted down by this area's startup state machine. */
extern u16 D_801448EE;
/* @source 0x80143BB0 @kind unknown — shared phase byte this area waits on. */
extern u8 D_80143BB0;
/* @source 0x80146867 @kind unknown — shared counter byte this area waits on
 * before incrementing it. */
extern u8 D_80146867;
/* @source 0x80144F28 @kind unknown — shared block released by this area. */
extern u8 D_80144F28;
/* @source 0x801F3AB4 @kind table */
extern void (*handlerTable[])(void);
/* @source 0x801F3ABC @kind table */
extern void (*D_801F3ABC[])(void);

void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
u8   func_8019601C(void);
s32  func_80155560(u32 arg0, void* arg1, s32 arg2);
s32  func_8015B5D4(u32 arg0, s32 arg1);
void func_8017AA94(void* arg0);
void func_8014E5A0(u8 arg0, u8 arg1);

/* @source 0x8017B2B4 — PsyQ LIBGPU graphics-type query; the sibling world
 * targets declare the same prototype (it is absent from the shipped headers). */
s32  GetGraphType(void);

/* @source 0x801783C8 — shared 12-bit angular helper (cosine companion): the
 * trail projector scales the record radius for the projected x component. */
s32  func_801783C8(s32 arg0);
/* @source 0x801782FC — companion 12-bit angular helper for the y component. */
s32  func_801782FC(s32 arg0);
/* @source 0x801AFE18 — shared projection setup consuming a 0x20-byte stack
 * matrix; no argument register other than a0 is live at the call site. */
void func_801AFE18(void* arg0);
/* @source 0x801AFF04 — projects the passed vector into the passed screen pair. */
void func_801AFF04(const void* arg0, void* arg1);

/* @source 0x8015DF18 @kind unknown — shared frontend cue dispatcher called by
 * the local work-table seeder func_801F2C74 with cue id 0x206. */
void func_8015DF18(u16 arg0);

/* @source 0x8015C088 — shared front-end startup helper run by this area.
 * @source 0x8015C058 — shared front-end release helper run by this area. */
void func_8015C088(void);
void func_8015C058(void);
/* @source 0x8015B580 — shared block release helper (owner, bit). */
void func_8015B580(void* arg0, u8 bit_index);
/* @source 0x8015B5A8 — shared block bit-clear helper (owner, bit). */
void func_8015B5A8(void* arg0, u8 bit_index);
/* @source 0x801BE1B0 — shared front-end helper taking the step selector. */
void func_801BE1B0(u32 arg0);
/* @source 0x8019FA28 — shared front selector context request
 * (seed, context_a, context_b, kind). */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801A4BC0 — shared world-marker queue: the sibling world targets
 * declare the same prototype (it is absent from the shipped headers). */
void func_801A4BC0(s16 x, s16 y, u32 size);

void func_801F2E3C(void* arg0);
void emitTrailStrip(const void* arg0);
/* @source 0x80196070 — shared work-area reset helper this area's work-trail
 * stepper calls when its scratch-work countdown reaches zero. */
void func_80196070(void);
void func_801F304C(void* arg0);
void emitMarkerPair(void);
void func_801F33A8(void);
POLY_FT4* emitMarkerQuad(const void* arg0, s32 arg1, u32 arg2);
void func_801F3618(void);
void func_801F3650(void);
void func_801F3690(void);
void func_801F36D0(void);
void func_801F3710(void);
/* @source 0x801F376C — queues the 2x3 shared world-marker block of this area. */
void func_801F376C(void);
/* @source 0x801F37E4 — runs this area's five-step shared front-end startup. */
void func_801F37E4(void);

#define WORLD00_AREA027_WORK_BASE     PSX_PTR(World00Area027Work, 0x800e4800u)
#define WORLD00_AREA027_PRIMITIVE_PTR PSX_REF(volatile u8*, 0x8014598cu)
#define WORLD00_AREA027_SCRATCH_PTR   D_1F800044
#define WORLD00_AREA027_MATRIX_92E8   (&D_801492E8)
#define WORLD00_AREA027_FLAG_48EB     PSX_REF(u8, 0x801448ebu)
#define WORLD00_AREA027_FLAG_48EC     PSX_REF(u8, 0x801448ecu)
#define WORLD00_AREA027_STATE_90A8    PSX_REF(u16, 0x801490a8u)
#define WORLD00_AREA027_FLAGS_90C7    PSX_REF(s8, 0x801490c7u)

#endif

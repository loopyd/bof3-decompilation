#ifndef EMI_WORLD00_AREA026_13_INTERNAL_H
#define EMI_WORLD00_AREA026_13_INTERNAL_H

#include "bof3/bof3.h"

/* Screen-projected point pair used by the area's rotating ring: the eight-byte
 * stride is proven by func_801F2E04 (projected centre at +0x00, current ring
 * point at +0x08 of one address-taken local). */
typedef struct World00Area026ScreenPoint {
  u16 x;
  u16 y;
  u16 z;
  u16 pad;
} World00Area026ScreenPoint;

/* Front-end record table at 0x80143FC8 — 0x74 bytes per record (the original
 * scales the allocator index by `0x74`). Only the occupancy byte at 0x00, the
 * byte at 0x05 and the geometry words at 0x34/0x38/0x3C are proven for this
 * target. */
typedef struct World00Area026Record {
  u8  flags_00;
  u8  unk_01[4];
  u8  unk_05;
  u8  pad_06[0x2e]; /* 0x06 - 0x33 */
  s32 unk_34;
  s32 unk_38;
  s32 unk_3C;
  u8  pad_40[0x34]; /* 0x40 - 0x73 */
} World00Area026Record;

/* @source 0x80143FC8 @kind bss — front-end record table holding the area's
 * spawned effect/trail records. */
extern World00Area026Record D_80143FC8[];
/* @source 0x8014686C @kind unknown */
extern volatile u32 D_8014686C;
/* @source 0x8014932A @kind bss — area counter; stepped by +/-0x14 via the
 * counter advance/retreat pair and cleared by resetCounter. */
extern volatile u16 selectionCounter;
/* @source 0x80149333 @kind unknown */
extern volatile u8  D_80149333;
/* @source 0x80146864 @kind unknown */
extern volatile u8 g_ScenarioProgress;
/* @source 0x801490A8 @kind unknown */
extern u16 D_801490A8;
/* @source 0x801490C7 @kind unknown */
extern s8 D_801490C7;
/* @source 0x801F4CDC @kind table */
extern u16 D_801F4CDC[];
/* @source 0x80144F5A @kind unknown */
extern u8 D_80144F5A;
/* @source 0x80145E90 @kind unknown — work base published as the scratchpad
 * work cursor by func_801F32B8. */
extern u8 D_80145E90[];
/* @source 0x80145EBE @kind unknown */
extern s16 D_80145EBE;
/* @source 0x80145EC0 @kind unknown */
extern s16 D_80145EC0;
/* @source 0x801F4CE0 @kind table */
extern u8 D_801F4CE0[];
/* @source 0x801F33EC @kind table — the overlay's entry-mode dispatch table:
 * the entry dispatcher selects one handler with the scratch cursor's mode byte
 * at offset 1. */
extern void (*D_801F33EC[])(void);
/* @source 0x801F33FC @kind table — four signed ring x offsets (1, -1, -1, 1)
 * stepped with the four ring angles by func_801F2D5C. */
extern s32 D_801F33FC[];
/* @source 0x801F340C @kind table — four signed ring y offsets (1, 1, -1, -1)
 * paired with D_801F33FC by func_801F2D5C. */
extern s32 D_801F340C[];
/* @source 0x1F800044 @kind unknown — scratchpad work cursor. */
extern u8* D_1F800044;

void game_stop_selection_fx(u32 effect_group, s32 effect_id);
void func_8015B580(void* arg0, u8 bit_index);
void func_8015B5A8(void* arg0, u8 bit_index);
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);
/* @behavior shared scenario reward gate: reports the state of the (group, id)
 * pair selected by the first two bytes of the four-byte ability id.
 * @source 0x801650B4
 */
u8 func_801650B4(u8 arg0, u8 arg1, u8 arg2, u8 arg3);
/* @source 0x8015DF18 @kind unknown — shared frontend cue dispatcher called by
 * the work-cursor service step with cue id 0x20D. */
void func_8015DF18(u16 arg0);

/* @source 0x8019601C — front-end record allocator: returns the index of the
 * next free 0x74-byte record at 0x80143FC8, or 0xFF when the table is full. */
u8 func_8019601C(void);
/* @source 0x8015477C — shared distance helper over the word pair passed as
 * (x, z); the result is published as a 0x10000-scaled record word. */
s16 func_8015477C(s32 arg0, s32 arg1);

void copyWorkareaFieldsAndAdvanceMode(void); /* @source 0x801F2C48 */
/* @source 0x801AFE18 — shared projection setup consuming a 0x20-byte stack
 * scratch area; only a0 is live at the call site. */
void func_801AFE18(void* arg0);
/* @source 0x801F2D5C @kind unknown — local four-entry ring dispatcher fed the
 * work-cursor record centre block at +0xC and its +0x1C scale word. */
void func_801F2D5C(u32* arg0, u32 arg1);
/* @source 0x8017A954 — shared libgpu POLY_F3 setter (four-word packet, code
 * 0x20) verified in the shipped executable; absent from the SDK map, so it
 * stays target-local. */
void func_8017A954(void* arg0);
/* @source 0x801AFF04 — shared projector from a three-word point to a screen
 * halfword pair. */
void func_801AFF04(const void* arg0, void* arg1);
/* @source 0x801782FC/0x801783C8 — shared angular helpers. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);
/* @source 0x80155A08 — shared grid-cell primitive linker: links the current
 * primitive cursor into the cell list and advances it by the passed size. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801F2E04(VECTOR* center, s32 scale, s32 start_angle, s32 xofs,
                   s32 yofs);
void func_801F300C(void);
void func_801F305C(void);

void setScenarioFlagBit29ClearBit28(void);
void clearScenarioFlagBit29(void);
void setScenarioFlagBit28(void);
void clearScenarioFlagBit28(void);

#endif

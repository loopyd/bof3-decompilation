#ifndef EMI_WORLD00_AREA028_13_INTERNAL_H
#define EMI_WORLD00_AREA028_13_INTERNAL_H

#include "bof3/bof3.h"

#include <rand.h>

typedef struct World00Area028Work {
  u8  unk_00[4];
  s16 field_04;
  s16 field_06;
  s16 scale;
  s16 unk_0a;
  s16 field_0c;
  s16 field_0e;
} World00Area028Work;

typedef struct Area028SpriteSlot {
  u8  unk_00;
  u8  unk_01;
  u8  pad_02[0x2c];
  s16 unk_2e;
  u8  pad_30[4]; /* 0x30 - 0x33 */
  s32 unk_34;
  s32 unk_38;
  s32 unk_3C;
} Area028SpriteSlot;

/* One 4-byte screen-space pair of the rotation trail: two halfwords, so the
 * original copies whole entries with the unaligned lwl/lwr + swl/swr idiom. */
typedef struct World00Area028ScreenPoint {
  u16 x;
  u16 y;
} World00Area028ScreenPoint;

/* Front-end record table at 0x80143FC8 — 0x74 bytes per record. Only the
 * occupancy byte at 0x00, the byte at 0x05 and the geometry words at
 * 0x34/0x38/0x3C are proven for this target. */
typedef struct World00Area028Record {
  u8  flags_00;
  u8  unk_01[4];
  u8  unk_05;
  u8  pad_06[0x2e]; /* 0x06 - 0x33 */
  s32 unk_34;
  s32 unk_38;
  s32 unk_3C;
  u8  pad_40[0x34]; /* 0x40 - 0x73 */
} World00Area028Record;

/* @source 0x1F800044
 * @kind unknown — scratchpad cell holding the pointer to the current AREA028
 * work record. Declared as a named symbol: the original reaches the cell with a
 * fresh folded lui+lw per access, which the raw-constant form cannot express
 * (the compiler keeps one shared address register across the calls). */
extern Area028SpriteSlot*    D_1F800044;

/* @source 0x80146864
 * @kind data — current scenario progress byte; the AREA028 entry handlers gate
 * on its value. Non-volatile: the original reads it with a plain lbu (a volatile
 * declaration adds a redundant andi 0xff at the comparison). */
extern u8                    g_ScenarioProgress;

/* @source 0x801F3E00
 * @kind bss — current AREA028 work table cursor; seeded from the work table
 * base and walked by the seed/scan/refresh helpers. */
extern World00Area028Work*   workCursor;

/* @source 0x80143FC8 @kind bss */
extern World00Area028Record D_80143FC8[];

void func_801AFE18(void* arg0);
void func_80196070(void);
void func_801AFF04(const void* arg0, void* arg1);
u16  func_8017A620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8017A97C(void* arg0);

/* @source 0x801782FC — shared angular helper; the trail's y step rotates the
 * passed radius through it. */
s32  func_801782FC(s32 arg0);
/* @source 0x801783C8 — companion helper for the trail's x step. */
s32  func_801783C8(s32 arg0);
void func_8017A904(void* arg0, s32 arg1);
void func_8017AA30(void* arg0);
void func_8017C2D8(void* arg0, s32 arg1, s32 arg2, s32 arg3, void* arg4);
void func_8014E5A0(u8 arg0, u8 arg1);

/* Front-end record allocator: returns the index of the next free 0x74-byte
 * record at 0x80143FC8, or 0xFF when the table is full. */
u8   func_8019601C(void);

/* Shared distance helper over the word pair passed as (x, z). */
s16  func_8015477C(s32 arg0, s32 arg1);

void  func_801F2D3C(void);
void  seedWorkTable(void);
void  initSpriteSlot(void* arg0);
void* scanFreeSlot(void);
void  refreshSpriteSlots(void);
void  func_801F318C(s16 arg0);

#define WORLD00_AREA028_WORK_PTR (workCursor)
#define WORLD00_AREA028_WORK_BASE     ((World00Area028Work*)0x800e4800u)
#define WORLD00_AREA028_PRIMITIVE_PTR PSX_REF(volatile u8*, 0x8014598cu)
/* The 33-entry halfword-pair trail at 0x800E4A00: entry 0 is the record's own
 * projected centre, entries 1..32 are the ring positions. */
#define WORLD00_AREA028_TRAIL ((World00Area028ScreenPoint*)0x800e4a00u)
#define WORLD00_AREA028_RING_X(index)                                          \
  PSX_REF(volatile u16, 0x800e4a04u + ((u32)(index) * 4u))
#define WORLD00_AREA028_RING_Y(index)                                          \
  PSX_REF(volatile u16, 0x800e4a06u + ((u32)(index) * 4u))

/* Byte-offset views of the same two ring tables, for readers that already hold
 * the scaled entry offset in a register: binding the scaled offset to a local
 * keeps the table base in the load displacement (lui $at,%hi; addu $at,off,$at;
 * lhu %lo($at)), which the inline `base + index * 4` form cannot express. */
#define WORLD00_AREA028_RING_X_AT(offset)                                      \
  PSX_REF(volatile u16, 0x800e4a04u + (u32)(offset))
#define WORLD00_AREA028_RING_Y_AT(offset)                                      \
  PSX_REF(volatile u16, 0x800e4a06u + (u32)(offset))

#endif

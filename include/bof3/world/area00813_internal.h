#ifndef EMI_WORLD00_AREA008_13_INTERNAL_H
#define EMI_WORLD00_AREA008_13_INTERNAL_H

#include "bof3/bof3.h"
#include "gpu/prim.h"

typedef struct World00Area008Scratch {
  u8 unk_00[0x5d];
  s8 field_5d;
  s8 field_5e;
} World00Area008Scratch;

typedef struct World00Area008State {
  u8 unk_00;
  u8 mode;
  u8 unk_02[0x7];
  u8 unk_09;
  u8 unk_0a;
  u8 entityIndex;
  u8 unk_0C[0x10c]; /* 0x0C - 0x117; the work-entity records reach 0x98, so
                     * this tail is proven only for the local area state
                     * block at 0x80145FD0. */
  u8 unk_118;       /* State byte 0x118 (the same address as D_801460E8):
                     * bit 0x40 is set when the area resource is installed
                     * and cleared when it is released. */
} World00Area008State;

typedef struct World00Area008Entity {
  u8 flags;
  u8 unk_01[0x49];
  u8 unk_4A;
  u8 unk_4B[0xd];
  u16 unk_58;
  u8 unk_5A[0x3e];
} World00Area008Entity;

/* Front-end record table at 0x80143FC8 — 0x74 bytes per record. Only the
 * occupancy byte at 0x00, the byte at 0x05 and the byte at 0x0B that receives
 * the work-entity index are proven for this target. */
typedef struct World00Area008Record {
  u8 flags_00;
  u8 unk_01[4];
  u8 unk_05;
  u8 unk_06[5];
  u8 unk_0B;
  u8 unk_0C[0x68]; /* 0x74 - 0x0C */
} World00Area008Record;

typedef void (*World00Area008Handler)(void);

/* @source 0x80143FC8 @kind bss */
extern World00Area008Record D_80143FC8[];
/* @source 0x1F800000 @kind unknown */
extern u8           D_1F800000;
/* @source 0x80146884 */
/* @kind: bss — work-entity cursor; holds the address of the selected entry of
 * D_80146888, so the pointer difference against that table is its index. */
extern World00Area008Entity* D_80146884;
/* @source 0x80146865 @kind unknown */
extern u8           D_80146865;
/* @source 0x80146866 @kind unknown */
extern u8           D_80146866;
/* @source 0x80146867 @kind unknown */
extern u8           D_80146867;
/* @source 0x80146876 @kind unknown */
extern u16          D_80146876;
/* @source 0x80146888 @kind unknown */
extern World00Area008Entity D_80146888[];
/* @source 0x80146C4C */
/* @kind: bss — area counter 3; stepped by +/-0x800 via the counter3 table
 * triple. */
extern s32          counter3;
/* @source 0x80149328 */
/* @kind: bss — area counter 1; stepped by +/-0x14 via the counter1 table
 * triple. */
extern volatile u16 counter1;
/* @source 0x80149333 @kind unknown */
extern volatile u8  D_80149333;
/* @source 0x8014932A */
/* @kind: bss — area counter 2; stepped by +/-0x14 via the counter2 table
 * triple. */
extern volatile u16 counter2;
/* @source 0x80145AD4 @kind unknown */
extern u8           D_80145AD4[];
/* @source 0x801F2C04 @kind unknown */
extern u8           D_801F2C04[];
/* @source 0x801F2C10 @kind unknown */
extern u8           D_801F2C10[];
/* @source 0x801F2C14 @kind unknown */
extern u8           D_801F2C14[];
/* @source 0x801F4674 @kind unknown */
extern u8           D_801F4674[];
/* @source 0x801F4680 @kind unknown */
extern u8           D_801F4680[];
/* @source 0x801F4688 */
/* @kind: table — per-mode handler pointers dispatched by
 * dispatchMode via the scratch state mode byte. */
extern World00Area008Handler modeHandlerTable[];
/* @source 0x801F46B0
 * @kind: table — 15 area-state mode handler pointers. */
extern World00Area008Handler areaStateModeHandlerTable[];
/* @source 0x801F46EC @kind unknown */
extern World00Area008Handler D_801F46EC[];

/* Shared primitive cursor (PsyQ SDK global, owned by the main exe). A named
 * symbol (not a fixed-address macro) so codegen emits the symbol-relative
 * `lui + lw reg, %lo(reg)` load the original binary uses. */
/* @source 0x8014598C @kind unknown */
extern u8* g_PrimCursor;
/* @source 0x1F800044 */
/* @kind: bss — scratchpad cell holding the current area state pointer. */
extern World00Area008State* g_areaWork;
/* @source 0x80145FD0 */
/* @kind: bss — local area state block installed as the current state. */
extern World00Area008State  areaState;
/* @source 0x80146250 */
/* @kind: bss — current area state pointer cell, written by the mode
 * handlers. */
extern World00Area008State* currentState;
/* @source 0x8014601A @kind unknown */
extern u8                   D_8014601A;
/* @source 0x80146028 @kind unknown */
extern u16                  D_80146028;
/* @source 0x80143BB0 @kind unknown */
extern u8                   D_80143BB0;
/* @source 0x801460E8 @kind unknown */
extern volatile u8          D_801460E8;
/* @source 0x801F53F4 */
/* @kind: bss — per-area countdown byte; loaded from small tables, decremented
 * by the mode handlers. */
extern volatile u8          countdown;
/* @source 0x80146864 @kind unknown */
extern volatile u8          g_ScenarioProgress;
/* @source 0x801490A8 @kind unknown */
extern u16                  D_801490A8;
/* @source 0x801490C7 @kind unknown */
extern s8                   D_801490C7;
/* @source 0x801490D8 @kind unknown */
extern u8                   D_801490D8[];
/* @source 0x801F53F0 @kind table */
extern u16                  D_801F53F0[];

u8   func_8019601C(void);
s32  GetGraphType(void);
s32  func_8017E3F4(char* buffer, const char* format, ...);
void func_8014FF0C(s16 arg0, s16 arg1, s32 arg2, const void* arg3);
void func_80150224(s32 arg0);
void func_801A4BC0(s16 x, s16 y, u32 size);
void func_801AEBA0(s16 arg0, s16 arg1, s16 arg2, s16 arg3, s32 arg4);

void drawScratchStatus(void);
void drawFlagStatus(void);
void func_801F2F24(void);
void func_801F317C(void);
void drawTexturedFrame(s32 arg0, s32 arg1, s32 arg2, s32 arg3, u8 arg4);
void func_801F3244(void);

#define WORLD00_AREA008_SCRATCH_PTR                                            \
  PSX_REF(volatile World00Area008State*, 0x1f800044u)

#endif

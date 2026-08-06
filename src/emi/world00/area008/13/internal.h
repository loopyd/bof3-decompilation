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
} World00Area008State;

typedef void (*World00Area008Handler)(void);

extern u8           WORLD00_AREA008_D_80146867;
/* @kind: bss — area counter 3; stepped by +/-0x800 via the counter3 table
 * triple. */
extern s32          world00_area008_counter3;
/* @kind: bss — area counter 1; stepped by +/-0x14 via the counter1 table
 * triple. */
extern volatile u16 world00_area008_counter1;
extern volatile u8  D_80149333;
/* @kind: bss — area counter 2; stepped by +/-0x14 via the counter2 table
 * triple. */
extern volatile u16 world00_area008_counter2;
extern u8           WORLD00_AREA008_D_80145AD4[];
extern u8           WORLD00_AREA008_D_801F2C04[];
extern u8           WORLD00_AREA008_D_801F2C10[];
/* @kind: table — per-mode handler pointers dispatched by
 * world00_area008_dispatch_mode via the scratch state mode byte. */
extern World00Area008Handler world00_area008_mode_handlerTable[];

/* Shared primitive cursor (PsyQ SDK global, owned by the main exe). A named
 * symbol (not a fixed-address macro) so codegen emits the symbol-relative
 * `lui + lw reg, %lo(reg)` load the original binary uses. */
extern u8* D_8014598C;
/* @kind: bss — scratchpad cell holding the current area state pointer. */
extern World00Area008State* g_world00_area008_work;
/* @kind: bss — local area state block installed as the current state. */
extern World00Area008State  world00_area008_state;
/* @kind: bss — current area state pointer cell, written by the mode
 * handlers. */
extern World00Area008State* world00_area008_currentState;
extern volatile u8          D_801460E8;
extern volatile u8          D_80146866;
/* @kind: bss — per-area countdown byte; loaded from small tables, decremented
 * by the mode handlers. */
extern volatile u8          world00_area008_countdown;

s32  func_8017E3F4(char* buffer, const char* format, ...);
void func_8014FF0C(s16 arg0, s16 arg1, s32 arg2, const void* arg3);
void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);
void func_801AEBA0(s16 arg0, s16 arg1, s16 arg2, s16 arg3, s32 arg4);

void world00_area008_draw_scratch_status(void);
void world00_area008_draw_flag_status(void);
void func_801F3D88(s16 arg0, s16 arg1, s32 arg2, s32 arg3, u8 arg4);

#define WORLD00_AREA008_SCRATCH_PTR                                            \
  PSX_REF(volatile World00Area008State*, 0x1f800044u)
#define WORLD00_AREA008_STATE_PTR                                              \
  PSX_REF(volatile World00Area008State*, 0x80146250u)
#define WORLD00_AREA008_STATE_BASE PSX_PTR(World00Area008State, 0x80145fd0u)

#endif

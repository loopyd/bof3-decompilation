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

extern volatile u8  WORLD00_AREA008_D_80146867;
extern s32          D_80146C4C;
extern volatile u16 D_80149328;
extern volatile u8  D_80149333;
extern volatile u16 WORLD00_AREA008_D_8014932A;
extern volatile u8  WORLD00_AREA008_D_80149333;
extern u8           WORLD00_AREA008_D_80145AD4[];
extern u8           WORLD00_AREA008_D_801F2C04[];
extern u8           WORLD00_AREA008_D_801F2C10[];

/* Shared primitive cursor (PsyQ SDK global, owned by the main exe). A named
 * symbol (not a fixed-address macro) so codegen emits the symbol-relative
 * `lui + lw reg, %lo(reg)` load the original binary uses. */
extern u8* D_8014598C;
extern World00Area008State* D_1F800044;
extern World00Area008State  D_80145FD0;
extern World00Area008State* D_80146250;
extern volatile u8          D_801460E8;
extern volatile u8          D_80146866;

s32  func_8017E3F4(char* buffer, const char* format, ...);
void func_8014FF0C(s16 arg0, s16 arg1, s32 arg2, const void* arg3);
void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);
void func_801AEBA0(s16 arg0, s16 arg1, s16 arg2, s16 arg3, s32 arg4);

void func_801F3C2C(void);
void func_801F3D18(void);
void func_801F3D88(s16 arg0, s16 arg1, s32 arg2, s32 arg3, u8 arg4);

#define WORLD00_AREA008_SCRATCH_PTR                                            \
  PSX_REF(volatile World00Area008State*, 0x1f800044u)
#define WORLD00_AREA008_STATE_PTR                                              \
  PSX_REF(volatile World00Area008State*, 0x80146250u)
#define WORLD00_AREA008_STATE_BASE PSX_PTR(World00Area008State, 0x80145fd0u)
#define WORLD00_AREA008_HANDLER_TABLE                                          \
  PSX_PTR(World00Area008Handler, 0x801f4688u)

#endif

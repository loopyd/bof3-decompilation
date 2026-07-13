#include "internal.h"

extern vu16 DAT_80143b90;
extern vu16 DAT_80143f00;
extern vu8  DAT_80143f1f;
extern vu8  DAT_80143f49;
extern vu8  DAT_80143f4a;
extern vu8  DAT_80143f4b;
extern vu16 DAT_801448fc;
extern vu8  DAT_801448ff;
extern vu32 DAT_80144900;
extern vu32 DAT_80144904;
extern vu32 DAT_80144fc0;
extern vu8  DAT_80145029;
extern vu8  DAT_80146256;
extern vu16 DAT_8014625a;
extern vu8  DAT_801462ea;
extern vu8  DAT_80146325;
extern vu8  DAT_80146880;
extern vu8  DAT_80146881;
extern vu8  DAT_8014832e;
extern vu8  DAT_8014933e;

void        func_8019fa28(u16 selection_seed, u32 context_a, u32 context_b,
                          u8 context_kind);
extern void func_801c1400(u32 arg0);

/* @behavior resets front-state globals, then seeds the authored selection byte from
 * the active EXE-side selection when one already exists.
 * @source 0x801970ec FUN_801970ec
 */
void func_801970ec(void) {
  u16 selection_seed;
  u16 world_flags;
  u32 context_a;
  u32 context_b;
  u8  context_kind;

  DAT_8014832e = 0x1fu;
  DAT_8014933e = 6u;
  func_801c1400(0u);
  selection_seed = DAT_801448fc;
  DAT_80143f00 = 0xffffu;
  world_flags = DAT_8014625a;
  context_a = DAT_80144900;
  context_b = DAT_80144904;
  context_kind = DAT_801448ff;
  DAT_80146256 = 0u;
  DAT_80143f49 = 0u;
  DAT_80143f4a = 0u;
  DAT_80143f4b = 0u;
  DAT_80146325 = 0u;
  DAT_801462ea = 0u;
  DAT_80146880 = 0u;
  DAT_80146881 = 0u;
  DAT_8014625a = world_flags | 0x4040u;
  func_8019fa28(selection_seed, context_a, context_b, context_kind);

  if (DAT_80144fc0 != 0u) {
    DAT_80143f1f = DAT_80145029;
  }

  DAT_80145029 = 0xffu;
  DAT_80143b90 = 1u;
}

#include "internal.h"

extern volatile u16 D_80143B90;
extern volatile u16 D_80143F00;
extern volatile u8  D_80143F1F;
extern volatile u8  D_80143F49;
extern volatile u8  D_80143F4A;
extern volatile u8  D_80143F4B;
extern volatile u16 D_801448FC;
extern volatile u8  D_801448FF;
extern volatile u32 D_80144900;
extern volatile u32 D_80144904;
extern volatile u32 D_80144FC0;
extern volatile u8  D_80145029;
extern volatile u8  D_80146256;
extern volatile u16 D_8014625A;
extern volatile u8  D_801462EA;
extern volatile u8  D_80146325;
extern volatile u8  D_80146880;
extern volatile u8  D_80146881;
extern volatile u8  D_8014832E;
extern volatile u8  D_8014933E;

void        func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                          u8 context_kind);
extern void func_801C1400(u32 arg0);

/* @behavior resets front-state globals, then seeds the authored selection byte from
 * the active EXE-side selection when one already exists.
 * @source 0x801970EC
 */
void func_801970EC(void) {
  u16 selection_seed;
  u16 world_flags;
  u32 context_a;
  u32 context_b;
  u8  context_kind;

  D_8014832E = 0x1fu;
  D_8014933E = 6u;
  CLOBBER_A0();
  func_801C1400(0u);
  selection_seed = D_801448FC;
  D_80143F00 = 0xffffu;
  world_flags = D_8014625A;
  context_a = D_80144900;
  context_b = D_80144904;
  context_kind = D_801448FF;
  D_80146256 = 0u;
  D_80143F49 = 0u;
  D_80143F4A = 0u;
  D_80143F4B = 0u;
  D_80146325 = 0u;
  D_801462EA = 0u;
  D_80146880 = 0u;
  D_80146881 = 0u;
  D_8014625A = world_flags | 0x4040u;
  func_8019FA28(selection_seed, context_a, context_b, context_kind);

  if (D_80144FC0 != 0u) {
    D_80143F1F = D_80145029;
  }

  D_80145029 = 0xffu;
  D_80143B90 = 1u;
}

#include "internal.h"

/* @behavior applies the pending front/world selection context, resolves the next
 * entry-0 mode from request flags, then advances to state 2.
 * @source 0x801971E8
 */
void applySelectionContext(void) {
  u16 context_seed;
  u32 context_a;
  u32 context_b;
  u8  context_kind;
  u16 world_flags;
  u16 final_world_flags;
  u8  request_kind;
  s32 pending_world;

  context_seed = D_80143F10;
  context_a = D_80143F14;
  world_flags = D_8014625A;
  context_b = D_80143F18;
  context_kind = D_80143F1C;
  D_8014625A = world_flags & 0xfff7u;
  func_8019FAA0(context_seed, context_a, context_b, context_kind);

  if (D_80143F02 & 8u) {
    D_80143B90 = 8u;
    return;
  }

  request_kind = D_80143F1D;
  if ((u8)(request_kind + 2u) >= 2u) {
    func_8014ECAC(request_kind);
    waitTransition(1u);
    if (*(u8*)&D_80143F02 & 1u) {
      func_801A0048(D_8014930A, D_8014930E);
    }
  } else if (request_kind == 0xfeu) {
    D_8014832E = 0u;
  }

  if (*(u8*)&D_80143F02 & 1u) {
    D_80143F1E = 10u;
  } else if (*(u16*)&D_80143F00 == 0xbdu) {
    D_80143F1E = 20u;
  } else {
    D_80143F1E = 0u;
  }

  pending_world = *(u16*)&D_80143F00;
  if (pending_world != 0xbd) {
    func_801B3CCC(1u);
  }

  final_world_flags = D_8014625A;
  D_80143BB0 = 0u;
  D_80143B90 = 2u;
  D_8014625A = final_world_flags & 0xffbfu;
}

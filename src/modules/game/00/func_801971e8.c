#include "internal.h"

/* @behavior applies the pending front/world selection context, resolves the next
 * entry-0 mode from request flags, then advances to state 2.
 * @source 0x801971e8 FUN_801971e8
 */
void func_801971e8(void) {
  u16 context_seed;
  u32 context_a;
  u32 context_b;
  u8  context_kind;
  u16 world_flags;
  u8  request_kind;
  s32 pending_world;

  context_seed = DAT_80143f10;
  context_a = DAT_80143f14;
  world_flags = DAT_8014625a;
  context_b = DAT_80143f18;
  context_kind = DAT_80143f1c;
  DAT_8014625a = world_flags & 0xfff7u;
  func_8019faa0(context_seed, context_a, context_b, context_kind);

  if (DAT_80143f02 & 8u) {
    DAT_80143b90 = 8u;
    return;
  }

  request_kind = DAT_80143f1d;
  if ((u8)(request_kind + 2u) >= 2u) {
    func_8014ecac(request_kind);
    func_80198bc4(1u);
    if (DAT_80143f02 & 1u) {
      func_801a0048(DAT_8014930a, DAT_8014930e);
    }
  } else if (request_kind == 0xfeu) {
    DAT_8014832e = 0u;
  }

  if (DAT_80143f02 & 1u) {
    DAT_80143f1e = 10u;
  } else if (DAT_80143f00 == 0xbd) {
    DAT_80143f1e = 20u;
  } else {
    DAT_80143f1e = 0u;
  }

  pending_world = DAT_80143f00;
  if (pending_world != 0xbd) {
    func_801b3ccc(1u);
  }

  DAT_80143bb0 = 0u;
  DAT_80143b90 = 2u;
  DAT_8014625a = DAT_8014625a & 0xffbfu;
}

#include "bof3/ui/game00_internal.h"

/* @behavior copies the active front selection/context into the shared cells
 *           D_80143F10 (seed), D_80143F14 and D_80143F18 (the two 16.16 context
 *           words) and D_80143F1C (kind), refreshes the pending request kind for
 *           that seed through func_801A02E8, resolves the request through
 *           func_801A0380 with the context words taken as whole units and the
 *           stored seed, then sets the front-end phase byte D_80143BB0 to 5.
 * @source 0x8019FA28
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind) {
  D_80143F10 = selection_seed;
  D_80143F14 = context_a;
  D_80143F18 = context_b;
  D_80143F1C = context_kind;
  func_801A02E8();
  func_801A0380((s32)context_a >> 16, (s32)context_b >> 16, D_80143F10);
  D_80143BB0 = 5;
}

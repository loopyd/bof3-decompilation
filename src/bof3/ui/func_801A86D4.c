#include "bof3/ui/game00_internal.h"

/* @behavior dispatches through the handler table at D_801C85F0 indexed by the
 * work byte at 0x7A, passing D_80144F28 as the second argument.
 * @source 0x801A86D4
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
s32 func_801A86D4(GameIndexedWork* work) {
  return D_801C85F0[work->handler_index_7A](work, D_80144F28);
}

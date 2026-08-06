#include "internal.h"

/* @behavior dispatches through the handler table at D_801C85F0 indexed by the
 * work byte at 0x7A, passing D_80144F28 as the second argument.
 * @source 0x801A86D4
 */
s32 func_801A86D4(GameIndexedWork* work) {
  return D_801C85F0[work->handler_index_7A](work, D_80144F28);
}

#include "bof3/ui/game00_internal.h"

/**
 * @source 0x8019A8B4
 * @behavior Dispatch the handler selected by work byte 1, then set state 2.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_8019A8B4(void) {
  struct GameWorkArea* work = SPAD_PTR_SLOT(struct GameWorkArea, 0x44u);

  D_801C80E0[work->unk_01]();
  D_80149333 = 2;
}

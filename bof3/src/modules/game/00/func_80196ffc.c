#include "internal.h"

/* does: requests START.EMI slot 0x268, waits for loader readiness, marks the
 * frontend pack dirty, then advances the local state.
 * @source: 0x80196ffc FUN_80196ffc
 */
void func_80196ffc(void) {
  emi_stream_init_slot(0x268u);

  while (!func_80162d00()) {
    func_8014b87c(1u);
  }

  func_8014e284();
  BOF3_GAME_PALETTE_STAGE_SERIAL += 1u;
  BOF3_GAME_ENTRY0_STATE += 1u;
}

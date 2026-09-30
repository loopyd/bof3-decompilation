#include "bof3/battle/battle15_internal.h"

/* @source 0x800ABBA4
 * @behavior Runs the battle mode pass func_800ABBE0 (which calls func_800ABF5C
 *   and then dispatches the mode handler D_800B5108[D_801462EA]) when the
 *   battle mode byte D_801462EA is nonzero, and the mode-zero reset/apply pass
 *   func_800ABC30 when it is clear.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchByBattleMode(void) {
  if (D_801462EA != 0) {
    func_800ABBE0();
  } else {
    func_800ABC30();
  }
}

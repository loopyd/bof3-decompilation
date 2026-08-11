#include "bof3/ui/game00_internal.h"

/* @behavior runs the preceding update, then starts the follow-up operation when
 * the active work region is zero and neither input status has bit 3 set.
 * @source 0x801C24D0
 * @status exact
 */
void func_801C24D0(void) {
  func_801C2538();

  if (g_game_work->field_05 == 0 &&
      ((D_80146258 | D_8014625A) & 8u) == 0) {
    func_801C2710();
  }
}

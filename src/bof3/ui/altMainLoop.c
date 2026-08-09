#include "bof3/ui/game00_internal.h"

/* @behavior alternate GAME.EMI entry-0 callback loop installed by the title
 * finalize path; resets local loop state, then dispatches through the entry-0
 * callback table.
 * @source 0x80196F78
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void altMainLoop(void) {
  const GameEntry0StateHandler* callbacks;

  D_80143B90 = 0u;
  D_80143B92 = 0u;
  func_8014BA04();
  func_80158E50();

  while (1) {
    callbacks = GAME_ALT_FRONT_CALLBACK_TABLE;
    func_8014B87C(1u);
    callbacks[D_80143B90]();
    func_80158C80();
  }
}

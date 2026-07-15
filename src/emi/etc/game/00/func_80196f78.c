#include "internal.h"

/* @behavior alternate GAME.EMI entry-0 callback loop installed by the title
 * finalize path; resets local loop state, then dispatches through the entry-0
 * callback table.
 * @source 0x80196f78 FUN_80196f78
 */
void func_80196f78(void) {
  const GameEntry0StateHandler* callbacks;

  D_80143B90 = 0u;
  D_80143B92 = 0u;
  func_8014ba04();
  func_80158e50();

  while (1) {
    callbacks = GAME_ALT_FRONT_CALLBACK_TABLE;
    func_8014b87c(1u);
    callbacks[D_80143B90]();
    func_80158c80();
  }
}

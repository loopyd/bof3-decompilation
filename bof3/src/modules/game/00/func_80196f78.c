#include "internal.h"

/* does: alternate GAME.EMI entry-0 callback loop installed by the title
 * finalize path; resets local loop state, then dispatches through the entry-0
 * callback table.
 * @source: 0x80196f78 FUN_80196f78
 */
void func_80196f78(void) {
  GAME_ENTRY0_STATE = 0u;
  GAME_ENTRY0_SUBSTATE = 0u;
  func_8014ba04();
  func_80158e50();

  while (1) {
    func_8014b87c(1u);
    BOF3_GAME_ALT_FRONT_CALLBACK_TABLE[GAME_ENTRY0_STATE]();
    func_80158c80();
  }
}

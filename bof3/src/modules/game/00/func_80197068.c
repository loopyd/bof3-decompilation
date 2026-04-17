#include "internal.h"

/* does: main GAME.EMI entry-0 selection callback loop installed by the title
 * boot path; dispatches through the authored-selection state table.
 * @source: 0x80197068 FUN_80197068
 */
void func_80197068(void) {
  BOF3_GAME_ENTRY0_STATE = 0u;
  BOF3_GAME_ENTRY0_SUBSTATE = 0u;
  func_8014ba04();
  func_80158e50();

  while (1) {
    BOF3_GAME_SELECTION_CALLBACK_TABLE[BOF3_GAME_ENTRY0_STATE]();
    func_80198cac();
    func_8014b87c(1u);
  }
}

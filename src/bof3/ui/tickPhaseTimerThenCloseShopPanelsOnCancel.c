#include "bof3/ui/shop00_internal.h"

/* @source 0x801E2474
 * @behavior shop sub-step handler reached as entry 1 of the three-entry
 *           sub-step table D_801E5D50 (the shipped word at 0x801E5D54 is this
 *           function), which the dispatcher func_801E2114 indexes with the UI
 *           sub-step byte D_80148652. Every call first republishes the panel
 *           state: it arms the s16 cell D_80148340 with the selected
 *           field-record id D_80148361 plus 0x122 and copies the list cursor
 *           byte D_8014835F (the byte the entry-0 sibling func_801E2150 steps
 *           with the pad) into the slot byte D_80148360. It then counts the
 *           shared frame timer phaseTimer down: a nonzero timer is decremented
 *           and the handler returns immediately. Once the timer reads zero and
 *           the pad state D_80145AA8 carries the cancel mask D_80145AC4, it
 *           plays the main-exe cues 0x106 and 0x102 through the cue dispatcher
 *           func_8015DF18, arms D_80148357 with 2, returns both 0x24-byte panel
 *           records anchored at D_80148378 to the state byte 1 that the opener
 *           func_801E2724 overwrites with 2 (D_8014837B and D_8014839F are that
 *           byte of the first and second record), re-arms the published slot
 *           byte D_80148360 with the 0x7F none sentinel that
 *           initializeShopUiState also uses, re-arms phaseTimer with 8 and
 *           advances the UI sub-step byte D_80148652 by one. A pad state
 *           without the cancel mask leaves the cue, panel, timer and sub-step
 *           cells untouched.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickPhaseTimerThenCloseShopPanelsOnCancel(void) {
  u8* timer;
  u8  value;

  D_80148340 = D_80148361 + 0x122;
  D_80148360 = D_8014835F;
  /* The original keeps one phaseTimer base in a callee-saved register across
   * both cue calls and fills the closing jump delay slot with the countdown
   * store, so the cell is viewed through a plain pointer here: the volatile
   * ordering barriers of the declared cell would keep that store out of the
   * delay slot. */
  timer = (u8*)&phaseTimer;
  value = *timer;
  if (value != 0) {
    *timer = value - 1;
    return;
  }
  if (D_80145AA8 & D_80145AC4) {
    func_8015DF18(0x106);
    func_8015DF18(0x102);
    D_80148357 = 2;
    D_8014837B = 1;
    D_8014839F = 1;
    D_80148360 = 0x7F;
    *timer = 8;
    D_80148652 += 1;
  }
}

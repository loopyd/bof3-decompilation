#include "bof3/battle/battle15_internal.h"

/* @source 0x8009A468
 * @behavior Queues event 0x104, clears the selection state byte 0x801462EF,
 * arms active selection slot byte 5, stages the active selection panel byte to
 * 2, advances the selection counter, marks byte 1 of the UI ring entry behind
 * the ring tail, runs panel tasks 16..19 and resets the selection substate
 * bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initSelectionStateSlot5(void) {
  u8 counter;
  u8 tail;

  func_8015DF18(0x104u);
  D_801462EF = 0;
  BATTLE_ACTIVE_SELECTION_SLOT_PTR[1] = 5;
  D_801EBF08_PTR->unk_01 = 2;
  counter = D_80146303;
  tail = D_801EC328;
  D_80146303 = counter + 1;
  D_801EB5B1[((tail - 1) & 0x0F) << 3] = 1;
  runPanelTasks16To19();
  D_801462E1 = 1;
  D_801462E2 = 0;
  D_801462E3 = 0;
  D_801462E4 = 0;
}

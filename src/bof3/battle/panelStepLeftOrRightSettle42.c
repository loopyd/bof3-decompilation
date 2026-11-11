#include "bof3/battle/battle15_internal.h"

/* Panel task as the panel x-step handlers read it: the byte at +0x0A lies past
 * the end of PanelTask and chooses which step arm runs here and in
 * func_800B0410. */
typedef struct BattlePanelStepTask {
  u8  unk_00[3];
  u8  state;
  s16 x;
  s16 field_06;
  u8  unk_08[2];
  u8  unk_0A;
} BattlePanelStepTask;

/* @source 0x800B0398
 * @behavior Runs the panel-task pass func_800B0498, then settles the panel
 * task x on the 0x42 rest coordinate. With byte +0x0A clear, a signed x below
 * 0x43 is reset to 0x42 and a larger x is stepped left by 0x20; with that byte
 * set, a signed x below 0x42 is stepped right by 0x20 and a larger x is reset
 * to 0x42.
 * @status exact
 * @match 100.00
 * @residual none
 */
void panelStepLeftOrRightSettle42(void) {
  BattlePanelStepTask* task;

  func_800B0498();
  task = (BattlePanelStepTask*)D_80148648;
  if (task->unk_0A == 0) {
    if (task->x < 0x43) {
      task->x = 0x42;
    } else {
      task->x -= 0x20;
    }
  } else {
    if (task->x < 0x42) {
      task->x += 0x20;
    } else {
      task->x = 0x42;
    }
  }
}

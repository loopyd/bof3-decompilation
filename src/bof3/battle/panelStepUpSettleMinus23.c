#include "bof3/battle/battle15_internal.h"

/* @source 0x800B0AB0
 * @behavior Steps the panel task y down by 0x10 while its signed value is at
 * least -0x16, otherwise resets it to -0x17, then draws the panel slots at x/y.
 * @status exact
 * @match 100.00
 * @residual none
 */
void panelStepUpSettleMinus23(void) {
    BattlePanelTask* task;
    BattlePanelTask* draw_task;
    s16 value;

    task = (BattlePanelTask*)D_80148648;
    value = task->field_06;
    if (value < -0x16) {
        task->field_06 = -0x17;
    } else {
        task->field_06 = value - 0x10;
    }
    draw_task = (BattlePanelTask*)D_80148648;
    func_800B0B0C((s16)draw_task->x, draw_task->field_06);
}

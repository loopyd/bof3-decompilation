#include "bof3/battle/battle15_internal.h"

/* @source 0x800B0A54
 * @behavior Steps the panel task y by 0x10 while its signed value is below
 * 0x29, otherwise resets it to 0x29, then draws the panel slots at its x/y.
 * @status exact
 * @match 100.00
 * @residual none
 */
void panelStepDownSettle29(void) {
    BattlePanelTask* task;
    BattlePanelTask* draw_task;
    s16 value;

    task = (BattlePanelTask*)panelTaskRoot;
    value = task->field_06;
    if (value < 0x29) {
        task->field_06 = value + 0x10;
    } else {
        task->field_06 = 0x29;
    }
    draw_task = (BattlePanelTask*)panelTaskRoot;
    drawBattlePanelSlots((s16)draw_task->x, draw_task->field_06);
}

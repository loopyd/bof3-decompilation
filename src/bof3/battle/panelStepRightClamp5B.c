#include "bof3/battle/battle15_internal.h"

/* Use the declared root cell so both original pointer loads remain explicit. */
#undef D_80148648

/* @source 0x800B0F30
 * @behavior Runs the panel-task pass func_800B0F9C, steps signed panel x right
 * by 0x20 while it is below 0x5B, then clamps a signed x of 0x5C or more back
 * to 0x5B.
 * @status exact
 * @match 100.00
 * @residual none
 */
void panelStepRightClamp5B(void) {
    PanelTask* task;
    PanelTask* clamp_task;
    s16 value;

    func_800B0F9C();
    task = D_80148648;
    value = task->x;
    if (value < 0x5B) {
        task->x = value + 0x20;
    }
    clamp_task = D_80148648;
    if ((s16)clamp_task->x >= 0x5C) {
        clamp_task->x = 0x5B;
    }
}

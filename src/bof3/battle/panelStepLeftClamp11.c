#include "bof3/battle/battle15_internal.h"

/* Use the declared root cell so both original pointer loads remain explicit. */
#undef D_80148648

/* @source 0x800B0EC4
 * @behavior Runs the panel-task pass func_800B0F9C, steps signed panel x left
 * by 0x20 when it is at least 0x12, then clamps signed x to at least 0x11.
 * @status exact
 * @match 100.00
 * @residual none
 */
void panelStepLeftClamp11(void) {
    PanelTask* task;
    PanelTask* clamp_task;
    s16 value;

    func_800B0F9C();
    task = D_80148648;
    value = task->x;
    if (value >= 0x12) {
        task->x = value - 0x20;
    }
    clamp_task = D_80148648;
    if ((s16)clamp_task->x < 0x11) {
        clamp_task->x = 0x11;
    }
}

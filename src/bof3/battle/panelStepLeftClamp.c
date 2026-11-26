#include "bof3/battle/battle15_internal.h"

/* Use the declared root cell so both original pointer loads remain explicit. */
/* @source 0x800B1664
 * @behavior Runs the panel-task pass func_800B1728, steps signed panel x left
 * by 0x20 when it is at least 0xA4, then clamps a signed x below 0xA3 up to
 * 0xA3.
 * @status exact
 * @match 100.00
 * @residual none
 */
void panelStepLeftClamp(void) {
    PanelTask* task;
    PanelTask* clamp_task;
    s16 value;

    func_800B1728();
    task = panelTaskRoot;
    value = task->x;
    if (value >= 0xA4) {
        task->x = value - 0x20;
    }
    clamp_task = panelTaskRoot;
    if ((s16)clamp_task->x < 0xA3) {
        clamp_task->x = 0xA3;
    }
}

#include "bof3/battle/battle15_internal.h"

/* @source 0x800B14BC
 * @behavior Runs the panel-task pass func_800B1728, then settles the panel
 * task x on the 0x5B rest coordinate: while the signed x is below 0x5C x is
 * reset to 0x5B, otherwise the value stepped left by 0x20 is stored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void panelStepLeftSettle5B(void) {
    PanelTask* task;
    s16 value;

    func_800B1728();
    task = D_80148648;
    value = task->x;
    if (value < 0x5C) {
        task->x = 0x5B;
    } else {
        task->x = value - 0x20;
    }
}

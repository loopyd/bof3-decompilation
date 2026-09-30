#include "bof3/battle/battle15_internal.h"

/* @source 0x800B0E24
 * @behavior Runs the panel-task pass func_800B0F9C, then settles the panel
 * task x on the 0x5B rest coordinate: while the signed x is below 0x5B the
 * value stepped right by 0x20 is stored, otherwise x is reset to 0x5B.
 * @status exact
 * @match 100.00
 * @residual none
 */
void panelStepRightSettle5B(void) {
    PanelTask* task;
    s16 value;

    func_800B0F9C();
    task = D_80148648;
    value = task->x;
    if (value < 0x5B) {
        task->x = value + 0x20;
    } else {
        task->x = 0x5B;
    }
}

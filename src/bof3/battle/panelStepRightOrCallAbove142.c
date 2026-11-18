#include "bof3/battle/battle15_internal.h"

/* @source 0x800B16D0
 * @behavior Runs the panel-task pass func_800B1728, then steps the signed
 * panel x right by 0x20 while it is below 0x143 and runs func_80158E20 in
 * place of that step once the signed x has reached 0x143. The payload holds
 * this body's address in the handler word at 0x80096A48, inside the
 * panel-state handler run that starts at 0x80096A34.
 * @status exact
 * @match 100.00
 * @residual none
 */
void panelStepRightOrCallAbove142(void) {
    PanelTask* task;
    s16 value;

    func_800B1728();
    task = D_80148648;
    value = task->x;
    if (value >= 0x143) {
        func_80158E20();
    } else {
        task->x = value + 0x20;
    }
}

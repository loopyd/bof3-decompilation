#include "bof3/battle/battle15_internal.h"

/* @source 0x800B1504
 * @behavior Runs the panel-task pass func_800B1728, then steps the signed
 * panel x right by 0x20 while it is below 0x15B and runs func_80158E20 in
 * place of that step once the signed x has reached 0x15B. Published as
 * handler 2 of the panel-state table at 0x80096A34.
 * @status exact
 * @match 100.00
 * @residual none
 */
void panelStepRightCall(void) {
    PanelTask* task;
    s16 value;

    func_800B1728();
    task = panelTaskRoot;
    value = task->x;
    if (value >= 0x15B) {
        func_80158E20();
    } else {
        task->x = value + 0x20;
    }
}

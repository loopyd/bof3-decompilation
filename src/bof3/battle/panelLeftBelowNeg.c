#include "bof3/battle/battle15_internal.h"

/* @source 0x800B0E6C
 * @behavior Runs the panel-task pass func_800B0F9C, then steps the signed
 * panel x left by 0x20 while it is at least -0xA4 and runs func_80158E20 in
 * place of that step once the signed x has fallen below -0xA4. Published as
 * handler 2 of the panel-task state table at 0x80096A14.
 * @status exact
 * @match 100.00
 * @residual none
 */
void panelLeftBelowNeg(void) {
    PanelTask* task;
    s16 value;

    func_800B0F9C();
    task = panelTaskRoot;
    value = task->x;
    if (value < -0xA4) {
        func_80158E20();
    } else {
        task->x = value - 0x20;
    }
}

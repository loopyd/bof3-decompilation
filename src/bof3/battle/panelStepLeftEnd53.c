#include "bof3/battle/battle15_internal.h"

/*
 * @source 0x800B21D8
 * @behavior Subtracts 0x20 from PanelTask x; if signed x falls below 0x53, sets x=0x52 and state=0.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void panelStepLeftEnd53(void) {
    PanelTask* task;

    task = D_80148648;
    task->x -= 0x20;
    if ((s16)task->x < 0x53) {
        *(s16*)&task->x = 0x52;
        task->state = 0;
    }
}

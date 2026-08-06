#include "internal.h"

/* @source 0x800B2198
 * @behavior adds 0x20 to PanelTask x; if signed x reaches or exceeds 0x54,
 * sets x to 0x52 and state to 0.
 */
void panelStepRightEnd54(void) {
    PanelTask* task;

    task = D_80148648;
    task->x += 0x20;
    if ((s16)task->x >= 0x54) {
        *(s16*)&task->x = 0x52;
        task->state = 0;
    }
}

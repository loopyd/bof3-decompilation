#include "internal.h"

/* @source 0x800B2158
 * @behavior subtracts 0x20 from PanelTask x; if signed x falls below -0xC8,
 * sets x to -0xC8 and state to 0.
 */
void func_800B2158(void) {
    PanelTask* task;

    task = D_80148648;
    task->x -= 0x20;
    if ((s16)task->x < -0xC8) {
        *(s16*)&task->x = -0xC8;
        task->state = 0;
    }
}

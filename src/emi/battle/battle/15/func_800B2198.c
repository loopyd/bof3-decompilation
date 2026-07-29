#include "internal.h"

void func_800B2198(void) {
    PanelTask* task;

    task = D_80148648;
    task->x += 0x20;
    if ((s16)task->x >= 0x54) {
        *(s16*)&task->x = 0x52;
        task->state = 0;
    }
}

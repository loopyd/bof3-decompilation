#include "internal.h"

void func_800B2158(void) {
    PanelTask* task;

    task = D_80148648;
    task->x -= 0x20;
    if ((s16)task->x < -0xC8) {
        *(s16*)&task->x = -0xC8;
        task->state = 0;
    }
}

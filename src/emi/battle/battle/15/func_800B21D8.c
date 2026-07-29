#include "internal.h"

void func_800B21D8(void) {
    PanelTask* task;

    task = D_80148648;
    task->x -= 0x20;
    if ((s16)task->x < 0x53) {
        *(s16*)&task->x = 0x52;
        task->state = 0;
    }
}

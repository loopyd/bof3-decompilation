#include "internal.h"

void func_800B254C(void) {
    PanelTask* task = g_PanelTaskRoot;
    s16 val;

    val = task->x - 0x20;
    task->x = val;
    if (val < 0x98) {
        task->x = 0x98;
        task->state = 0;
    }
}

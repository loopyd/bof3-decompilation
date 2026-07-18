#include "internal.h"
#include "bof3/ui/panel_task.h"

/* @source 0x801E4460
 * @behavior advances the panel task field at offset 6 by 0x10, clamps to max 0xF0, and
 *         clears state when reached.
 */
void func_801E4460(void) {
    Bof3PanelTask* task_root;
    u16            next_val;

    task_root = D_80148648;
    next_val = (u16)((*(volatile u16*)((u8*)task_root + 6)) + 0x10);
    *(volatile u16*)((u8*)task_root + 6) = next_val;
    if ((s16)next_val >= 0xF1) {
        *(volatile u16*)((u8*)task_root + 6) = 0xF0;
        task_root->state = 0;
    }
}

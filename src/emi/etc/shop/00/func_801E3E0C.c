#include "internal.h"
#include "bof3/ui/panel_task.h"

/* @source 0x801E3E0C
 * @behavior advances the panel task x position by 0x20, clamps to max 0x96, and
 *         clears state when reached.
 */
void func_801E3E0C(void) {
    Bof3PanelTask* task_root;
    u16            next_val;

    task_root = D_80148648;
    next_val = (u16)(task_root->x + 0x20);
    task_root->x = next_val;
    if ((s16)next_val >= 0x97) {
        task_root->x = 0x96;
        task_root->state = 0;
    }
}

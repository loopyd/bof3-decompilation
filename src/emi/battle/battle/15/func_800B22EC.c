#include "internal.h"

/* @source 0x800B22EC
 * @behavior Moves the panel left by 32 pixels, clamping it to x=83 and clearing its state.
 */

void func_800B22EC(void) {
    PanelTask* panel;

    panel = D_80148648;
    panel->x -= 0x20;
    if ((s16)panel->x < 0x53) {
        panel->x = 0x53;
        panel->state = 0;
    }
}

#include "internal.h"

/* @behavior dispatches through the handler table indexed by the selector
 * byte `D_800F724C` (tail call via jalr, no arguments).
 * @source 0x800F500C
 */
void func_800F500C(void) {
    D_800F71F0[D_800F724C]();
}

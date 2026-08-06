#include "internal.h"

/* @behavior dispatches through the handler table indexed by the selector
 * byte `D_80144286` (tail call via jalr, no arguments).
 * @source 0x801DCC74
 */
void func_801DCC74(void) {
    D_801E22E4[D_80144286]();
}

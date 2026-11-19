#include "bof3/world/area03004_internal.h"

/* @behavior Calls the handler selected by shared selector byte D_80144286
 * through the local handler table, with no arguments.
 * @source 0x801DCC74
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchHandlerBySelectorByteWorld00Area03004(void) {
    D_801E22E4[D_80144286]();
}

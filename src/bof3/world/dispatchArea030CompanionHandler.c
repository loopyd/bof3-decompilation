#include "bof3/world/area03005_internal.h"

/* @behavior dispatches through the handler table indexed by the selector
 * byte `D_800F724C` (tail call via jalr, no arguments).
 * @source 0x800F500C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchArea030CompanionHandler(void) {
    D_800F71F0[D_800F724C]();
}

#include "bof3/ui/shop00_internal.h"

/* @source 0x801E2800
 * @behavior returns without observable side effects.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void panelNoop(void) {}

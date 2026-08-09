#include "bof3/core/slus_internal.h"

/* @source 0x8014AA04
 * @behavior returns without observable side effects.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void bootNoop(void) {}

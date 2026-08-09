#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D32D4
 * @behavior dispatches the handler selected by D_801D4286 from the
 * D_801D4240 table.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801D32D4(void) { D_801D4240[D_801D4286](); }

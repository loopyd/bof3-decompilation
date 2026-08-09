#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D278C
 * @behavior dispatches the handler selected by D_801D4286 through the
 * D_801D4204 function-pointer table.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801D278C(void) {
  D_801D4204[D_801D4286]();
}

#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D3924
 * @behavior dispatches the handler selected by D_801D4286 from the
 * D_801D4264 table.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D3924(void) { D_801D4264[D_801D4286](); }

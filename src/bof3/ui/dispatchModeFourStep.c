#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D32D4
 * @behavior dispatches the handler selected by D_801D4286 from the
 * D_801D4240 table.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchModeFourStep(void) { D_801D4240[D_801D4286](); }

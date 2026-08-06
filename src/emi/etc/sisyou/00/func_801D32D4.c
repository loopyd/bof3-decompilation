#include "internal.h"

/* @source 0x801D32D4
 * @behavior dispatches the handler selected by D_801D4286 from the
 * D_801D4240 table.
 */
void func_801D32D4(void) { D_801D4240[D_801D4286](); }

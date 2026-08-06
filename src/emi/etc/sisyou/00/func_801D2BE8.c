#include "internal.h"

/* @source 0x801D2BE8
 * @behavior dispatches the handler selected by D_801D4286 through the
 * D_801D421C function-pointer table.
 */
void func_801D2BE8(void) {
  D_801D421C[D_801D4286]();
}

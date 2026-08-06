#include "internal.h"

/* @source 0x801D25D8
 * @behavior dispatches the mode handler selected by modeIndex through the
 * D_801D41E0 function-pointer table.
 */
void func_801D25D8(void) {
  D_801D41E0[modeIndex]();
}

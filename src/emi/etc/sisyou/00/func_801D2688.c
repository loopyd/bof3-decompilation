#include "internal.h"

/* @source 0x801D2688
 * @behavior dispatches the handler selected by D_801D4286 through the
 * D_801D41FC function-pointer table.
 */
void func_801D2688(void) {
  D_801D41FC[D_801D4286]();
}

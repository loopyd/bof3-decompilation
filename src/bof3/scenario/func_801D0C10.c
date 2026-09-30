#include "bof3/scenario/sce10eff_internal.h"

#include "base/compiler.h"

/* @behavior Dispatches the scratchpad work byte at offset 1 through the
 * three-entry local handler table published at 0x801D0C04 (func_801D0C70,
 * func_801D0E64, func_801D0E9C).
 * @source 0x801D0C10
 * @status exact
 * @match 100.00
 * @residual none
 */
void NO_SIBLING_CALLS func_801D0C10(void) {
  void (*handlers[3])(void) = {func_801D0C70, func_801D0E64, func_801D0E9C};
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44u);
  handlers[work[1]]();
}

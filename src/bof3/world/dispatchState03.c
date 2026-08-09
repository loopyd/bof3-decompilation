#include "bof3/world/area01613_internal.h"

/* @behavior dispatches through the second local handler table selected by
 * scratchpad state byte `0x03`.
 * @source 0x801F368C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchState03(void) {
  World00Area016Scratch* scratch;

  scratch = WORLD00_AREA016_SCRATCH_PTR;
  D_801F512C[scratch->state_03]();
}

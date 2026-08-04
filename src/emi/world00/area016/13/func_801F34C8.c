#include "internal.h"

/* @behavior dispatches through the local handler table selected by scratchpad
 * state byte `0x02`.
 * @source 0x801F34C8
 */
void func_801F34C8(void) {
  World00Area016Scratch* scratch;

  scratch = WORLD00_AREA016_SCRATCH_PTR;
  WORLD00_AREA016_D_801F511C[scratch->state_02]();
}

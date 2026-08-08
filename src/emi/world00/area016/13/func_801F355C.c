#include "internal.h"

/**
 * @source 0x801F355C
 * @behavior Advances the scratch height, updates state after reaching the
 * threshold, then advances the state-02 step.
 */
void func_801F355C(void) {
  World00Area016Scratch* scratch;

  scratch = WORLD00_AREA016_SCRATCH_PTR;
  scratch->field_2e += 0x10;
  if (scratch->field_2e >= 0x10) {
    scratch->state_02++;
  }
  advanceState02Step();
}

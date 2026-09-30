#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F7350
 * @behavior Advances the per-frame work-object countdown after reseeding the
 * scratchpad work object's sprite through func_801F7134(1): while the object's
 * byte at 0x09 is non-zero it is decremented by one, and when it reads zero
 * the object's state byte at 0x01 is incremented and byte 0x09 is re-armed to
 * 0xFF so the same handler runs again. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F7350(void) {
  func_801F7134(1);

  if (D_1F800044[9] != 0) {
    D_1F800044[9]--;
  } else {
    D_1F800044[1]++;
    D_1F800044[9] = 0xFF;
  }
}

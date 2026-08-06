#include "internal.h"

/* @behavior initializes the second frontend callback bank, ticks its shared
 * update paths, and advances to the next sub-state.
 * @source 0x80197A60
 */
void bank2Init(void) {
  u16* state;

  func_801BEDD0();
  func_801A06D8();
  func_801992B8();
  state = &D_80143B92;
  (*state)++;
}

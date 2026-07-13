#include "internal.h"

/* @behavior initializes the second frontend callback bank, ticks its shared
 * update paths, and advances to the next sub-state.
 * @source 0x80197a60 func_80197a60
 */
void func_80197a60(void) {
  u16* state;

  func_801bedd0();
  func_801a06d8();
  func_801992b8();
  state = &DAT_80143b92;
  (*state)++;
}

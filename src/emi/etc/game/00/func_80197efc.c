#include "internal.h"

/* @behavior runs the second frontend bank's paired local updates, ticks the
 * shared paths, and advances to its final sub-state.
 * @source 0x80197efc func_80197efc
 */
void func_80197efc(void) {
  u16* state;

  func_801bf8e0();
  func_801bfac4();
  func_801a06d8();
  func_801992b8();
  state = &DAT_80143b92;
  (*state)++;
}

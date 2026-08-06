#include "internal.h"

/* @behavior runs the second frontend bank's paired local updates, ticks the
 * shared paths, and advances to its final sub-state.
 * @source 0x80197EFC
 */
void game_front_bank2_final_updates(void) {
  u16* state;

  func_801BF8E0();
  func_801BFAC4();
  func_801A06D8();
  func_801992B8();
  state = &D_80143B92;
  (*state)++;
}

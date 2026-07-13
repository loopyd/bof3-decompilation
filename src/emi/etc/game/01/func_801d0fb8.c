#include "internal.h"

/* @behavior while the EXE gate is active, updates the frontend prompt;
 * otherwise clears the prompt selection and returns the overlay to state zero.
 * @source 0x801d0fb8 FUN_801d0fb8
 */
void func_801d0fb8(void) {
  if (DAT_80143b40 == 0u) {
    DAT_80145024 = 0xffu;
    GAME_FRONT_STATE = 0u;
  } else {
    func_801d11e4();
  }
}

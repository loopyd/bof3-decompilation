#include "internal.h"

/* @behavior while the EXE gate is active, updates the frontend prompt;
 * otherwise clears the prompt selection and returns the overlay to state zero.
 * @source 0x801D0FB8
 */
void updatePrompt(void) {
  if (D_80143B40 == 0u) {
    D_80145024 = 0xffu;
    GAME_FRONT_STATE = 0u;
  } else {
    drawPrompt();
  }
}

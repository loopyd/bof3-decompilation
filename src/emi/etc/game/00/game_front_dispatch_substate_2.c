#include "internal.h"

/* @behavior dispatches the current front-end sub-state through the second local
 * state-handler table.
 * @source 0x80197A24
 */
void game_front_dispatch_substate_2(void) {
  u16 state;

  state = D_80143B92;
  barrier();
  D_801C7B54[state]();
}

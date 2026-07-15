#include "internal.h"

/* @behavior dispatches the current front-end sub-state through the first local
 * state-handler table.
 * @source 0x801975e4 func_801975e4
 */
void func_801975e4(void) {
  u16 state;

  state = D_80143B92;
  D_801C7B44[state]();
}

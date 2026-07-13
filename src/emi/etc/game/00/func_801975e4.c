#include "internal.h"

/* @behavior dispatches the current front-end sub-state through the first local
 * state-handler table.
 * @source 0x801975e4 func_801975e4
 */
void func_801975e4(void) {
  u16 state;

  state = DAT_80143b92;
  DAT_801c7b44[state]();
}

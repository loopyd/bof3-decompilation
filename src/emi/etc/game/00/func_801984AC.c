#include "internal.h"

/* @behavior dispatches the current front-end sub-state through the fourth local
 * state-handler table.
 * @source 0x801984AC
 */
void func_801984AC(void) {
  u16 state;

  state = D_80143B92;
  barrier();
  D_801C7B88[state]();
}

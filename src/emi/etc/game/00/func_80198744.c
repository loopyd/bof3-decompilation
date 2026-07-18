#include "internal.h"

/* @behavior dispatches the current front-end sub-state through the fifth local
 * state-handler table.
 * @source 0x80198744
 */
void func_80198744(void) {
  u16 state;

  state = D_80143B92;
  barrier();
  D_801C7B98[state]();
}

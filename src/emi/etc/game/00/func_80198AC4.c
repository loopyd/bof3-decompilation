#include "internal.h"

/* @behavior dispatches the current front-end sub-state through the seventh local
 * state-handler table.
 * @source 0x80198AC4
 */
void func_80198AC4(void) {
  u16 state;

  state = D_80143B92;
  barrier();
  D_801C7BB0[state]();
}

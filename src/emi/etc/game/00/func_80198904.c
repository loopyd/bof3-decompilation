#include "internal.h"

/* @behavior dispatches the current front-end sub-state through the sixth local
 * state-handler table.
 * @source 0x80198904
 */
void func_80198904(void) {
  u16 state;

  state = D_80143B92;
  barrier();
  D_801C7BA4[state]();
}

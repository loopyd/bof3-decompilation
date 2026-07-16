#include "internal.h"

/* @behavior dispatches the current front-end sub-state through its local
 * state-handler table.
 * @source 0x80198234
 */
void func_80198234(void) {
  GameEntry0StateHandler callback;
  u16                    state;

  state = D_80143B92;
  callback = D_801C7B7C[state];
  callback();
}

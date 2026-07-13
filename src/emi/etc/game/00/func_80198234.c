#include "internal.h"

/* @behavior dispatches the current front-end sub-state through its local
 * state-handler table.
 * @source 0x80198234 func_80198234
 */
void func_80198234(void) {
  GameEntry0StateHandler callback;
  u16                   state;

  state = DAT_80143b92;
  callback = DAT_801c7b7c[state];
  callback();
}

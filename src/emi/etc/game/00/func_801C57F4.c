#include "internal.h"

/* @behavior dispatches the current scenario sub-state through the local
 * state-handler table at 0x801CD568.
 * @source 0x801C57F4
 */
void func_801C57F4(void) {
  GameEntry0StateHandler callback;
  u8                     state;

  state = D_80143F49;
  callback = D_801CD568[state];
  barrier();
  callback();
}

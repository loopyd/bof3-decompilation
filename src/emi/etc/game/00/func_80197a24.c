#include "internal.h"

/* @behavior dispatches the current front-end sub-state through the second local
 * state-handler table.
 * @source 0x80197a24 func_80197a24
 */
void func_80197a24(void) {
  u16 state;

  state = D_80143B92;
  D_801C7B54[state]();
}

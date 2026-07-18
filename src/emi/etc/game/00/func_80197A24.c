#include "internal.h"

/* @behavior dispatches the current front-end sub-state through the second local
 * state-handler table.
 * @source 0x80197A24
 */
void func_80197A24(void) {
  u16 state;

  state = D_80143B92;
  __asm__ __volatile__("" : : : "memory");
  D_801C7B54[state]();
}

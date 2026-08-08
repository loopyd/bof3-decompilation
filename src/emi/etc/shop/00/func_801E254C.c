#include "internal.h"

/* @source 0x801E254C
 * @behavior decrements phaseTimer; when it reaches zero, subtracts 2 from
 *           the sub-step byte D_80148652.
 */
void func_801E254C(void) {
  volatile u8* p = &phaseTimer;
  u8 val = *p - 1;

  *p = val;
  if (val == 0) {
    D_80148652 -= 2;
  }
}

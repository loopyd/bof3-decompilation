#include "bof3/scenario/scena16_internal.h"

/* @behavior returns zero.
 * @source 0x801F83A0
 * @status invalid
 * @match 100.00
 * @residual hook parameter contract unresolved; zero-return behavior and native bytes exact
 */
s32 returnZero2(void) {
  return 0;
}

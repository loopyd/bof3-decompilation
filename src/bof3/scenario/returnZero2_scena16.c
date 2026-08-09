#include "bof3/scenario/scena16_internal.h"

/* @behavior returns zero.
 * @source 0x801F83A0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
s32 returnZero2(void) {
  return 0;
}

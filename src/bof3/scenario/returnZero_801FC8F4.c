#include "bof3/scenario/scena00_internal.h"

/* @behavior returns zero.
 * @source 0x801FC8F4
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
s32 returnZero(void) {
  return 0;
}

#include "bof3/scenario/scena16_internal.h"

/* @behavior returns zero for the exported two-coordinate hook.
 * @source 0x801F83A8
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 returnZero3(s32 x, s32 y) {
  return 0;
}

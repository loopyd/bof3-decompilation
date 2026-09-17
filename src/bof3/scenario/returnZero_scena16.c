#include "bof3/scenario/scena16_internal.h"

/* @behavior returns zero.
 * @source 0x801F8398
 * @status invalid
 * @match 100.00
 * @residual known callback-type incompatibility with Scena16RecordCallback; native bytes exact
 */
s32 returnZero(void) {
  return 0;
}

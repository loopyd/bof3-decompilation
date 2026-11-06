#include "bof3/scenario/scena16_internal.h"

/* @behavior returns zero. Declared with no parameters while the record callback type
 * Scena16RecordCallback takes (record, arg1); the body reads no argument, so the
 * call-site contract stays unresolved even though the emitted 8-byte body is exact.
 * @source 0x801F83A0
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 returnZero2(void) {
  return 0;
}

#include "bof3/scenario/scena16_internal.h"

/* @behavior returns immediately.
 * @source 0x801F8530
 * @status invalid
 * @match 100.00
 * @residual known callback-type incompatibility with Scena16RecordCallback; native bytes exact
 */
void noopRecordHandler(void) {
  return;
}

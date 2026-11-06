#include "bof3/scenario/scena16_internal.h"

/* @behavior returns immediately. Declared void() while the record callback table
 * stores it as Scena16RecordCallback, whose (record, arg1) parameters the body
 * ignores; the call-site contract stays unresolved even though the 8-byte body is
 * exact.
 * @source 0x801F8530
 * @status exact
 * @match 100.00
 * @residual none
 */
void noopRecordHandler(void) {
  return;
}

#include "bof3/scenario/scena16_internal.h"

/* @behavior Returns immediately without reading arguments. The record callback
 * type takes a record and context word; this definition retains the zero-argument
 * interface. Compatibility of those interfaces remains unverified.
 * @source 0x801F8530
 * @status exact
 * @match 100.00
 * @residual none
 */
void noopRecordHandler(void) {
  return;
}

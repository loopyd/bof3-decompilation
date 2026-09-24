#include "bof3/bof3.h"

/* @source 0x801CF858
 * @behavior Overlay accessor: loads the pointer published at 0x80146250, clears its byte at +299 and returns the pointer.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* getAndClearPlcharPlp27a00_801CF858(void) {
  u8* record = *(u8**)0x80146250;
  record[299] = 0;
  return record;
}

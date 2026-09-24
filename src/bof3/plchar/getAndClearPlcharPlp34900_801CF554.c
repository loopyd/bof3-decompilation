#include "bof3/bof3.h"

/* @source 0x801CF554
 * @behavior Overlay accessor: loads the pointer published at 0x80146250, clears its byte at +299 and returns the pointer.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* getAndClearPlcharPlp34900_801CF554(void) {
  u8* record = *(u8**)0x80146250;
  record[299] = 0;
  return record;
}

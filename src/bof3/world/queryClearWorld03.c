#include "bof3/bof3.h"

/* @source 0x801F3B14
 * @behavior Overlay accessor: loads the pointer published at 0x1F800044, clears its byte at +1 and returns the pointer.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* queryClearWorld03(void) {
  u8* record = *(u8**)0x1F800044;
  record[1] = 0;
  return record;
}

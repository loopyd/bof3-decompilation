#include "bof3/bof3.h"

/* @source 0x800C2234
 * @behavior Overlay accessor: loads the pointer published at 0x1F800044, clears its byte at +11 and returns the pointer.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* getAndClearBossBoss05516_800C2234(void) {
  u8* record = *(u8**)0x1F800044;
  record[11] = 0;
  return record;
}

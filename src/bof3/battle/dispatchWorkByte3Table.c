#include "bof3/battle/battle03_internal.h"

/* @source 0x801DFC70
 * @behavior loads the non-volatile scratchpad pointer cell at 0x44, uses byte 3
 * of its pointed-to local work as an index, and calls that handler-table entry.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchWorkByte3Table(void) {
  D_801EB1D4[battleWork[3]]();
}

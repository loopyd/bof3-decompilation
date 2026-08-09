#include "bof3/battle/battle03_internal.h"

/* @source 0x801E4AE8
 * @behavior loads the non-volatile scratchpad pointer cell at 0x1F800044,
 * then invokes the byte-2-selected handler from the local 0x801EB460 table.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchByte2TableB460(void) {
  D_801EB460[battleWork[2]]();
}

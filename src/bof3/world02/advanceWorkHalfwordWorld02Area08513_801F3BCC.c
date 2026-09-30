#include "bof3/bof3.h"

/* @source 0x801F3BCC
 * @behavior Overlay accessor: adds 0x18 to the halfword at +0x3E of the work
 *           record held in scratchpad pointer-slot 0x1F800044, stores the sum
 *           back and returns it; takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 advanceWorkHalfwordWorld02Area08513_801F3BCC(void) {
  u16* work;
  s32 value;

  work = SPAD_PTR_TABLE(u16)[0x11];
  value = work[0x1F];
  value += 0x18;
  work[0x1F] = value;
  return value;
}

#include "bof3/bof3.h"

void func_801F54B8(void);

/* @source 0x801F533C
 * @behavior Overlay step handler: loads the scratch work-record cursor from the
 *           scratchpad pointer cell 0x1F800044, writes the halfword 0x320 (800)
 *           to the record's +0x3E field, increments the record's +0x02 step
 *           byte, and then runs the local panel update at 0x801F54B8; takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkHalfword320AndIncrementWorkByte2World02Area07714_801F533C(void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  *(u16*)(work + 0x3E) = 0x320;
  work[2]++;
  func_801F54B8();
}

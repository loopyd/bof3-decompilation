#include "internal.h"

/* @behavior clears D_80147A58 and increments scratch-work byte 0x01.
 * @source 0x801F2DB4
 */
void func_801F2DB4(void) {
  u8* work;

  work = (u8*)WORLD00_AREA024_SCRATCH_PTR;
  D_80147A58 = 0;
  work[1]++;
}

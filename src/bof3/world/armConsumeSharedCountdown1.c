#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */

/* @source 0x801F3C70
 * @behavior Entry handler at index 0 of the byte-4 dispatch table at
 *           0x801F4DE4, whose index 1 names the phase mover at 0x801F3CB0 and
 *           which the local dispatcher at 0x801F3C2C selects from with the
 *           work record byte at 0x04: it reads the work record published at
 *           the scratchpad pointer cell 0x1F800044 once per store without
 *           keeping a base in a local, stores 0x10 into the record phase byte
 *           at 0x0A and then 1 into that record byte at 0x04, so the next
 *           dispatch of the same table reaches the phase mover. Finally it
 *           reads the shared work record published at 0x80146250 and subtracts
 *           two units from its halfword at 0x122. Takes no arguments, makes no
 *           call, keeps no frame and returns nothing; the value left in $v0 is
 *           the halfword it just stored as a side effect.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armConsumeSharedCountdown1(void) {
  u8* shared;

  D_1F800044[0x0a] = 0x10;
  D_1F800044[0x04] = 1;
  shared = D_80146250;
  *(u16*)(shared + 0x122) -= 2;
}

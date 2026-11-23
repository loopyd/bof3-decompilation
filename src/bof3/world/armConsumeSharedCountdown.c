#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */

/* @source 0x801F3B5C
 * @behavior Entry handler of the byte-4 dispatch table at 0x801F4DCC, whose
 *           index 0 names this routine and whose index 1 names 0x801F3B9C: it
 *           reads the work record published at the scratchpad pointer cell
 *           0x1F800044 once per store without keeping a base in a local, stores
 *           0x10 into the record phase byte at 0x0A and then 1 into the record
 *           byte at 0x04 that the local dispatcher
 *           dispatchStateByte4Table4dccWorld01Area06813 indexes that table
 *           with, so the next dispatch reaches the phase-mover at 0x801F3B9C.
 *           Finally it reads the shared work record published at 0x80146250 and
 *           subtracts two units from its halfword at 0x122. Takes no arguments,
 *           makes no call, keeps no frame and returns nothing; the value left in
 *           $v0 is the halfword it just stored as a side effect.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armConsumeSharedCountdown(void) {
  u8* shared;

  D_1F800044[0x0a] = 0x10;
  D_1F800044[0x04] = 1;
  shared = D_80146250;
  *(u16*)(shared + 0x122) -= 2;
}

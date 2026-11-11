#include "bof3/world/area03004_internal.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @behavior entry 1 of the local handler table at 0x801E1D80, which the
 *           dispatcher func_801D2BF0 indexes with the work record's unsigned step
 *           byte at +0x1: it clears the work record's unsigned bytes at +0x9,
 *           +0xA and +0xB and the halfword at +0x30, then increments that step
 *           byte, so the next frame dispatches the following entry
 *           func_801D2DC4. The work record is published at the scratchpad
 *           pointer cell 0x1F800044; that cell is read once for the +0x9 and
 *           +0x30 clears, read again for the +0xA clear, and read once more for
 *           each of the +0xB clear and the +0x1 increment. The function takes
 *           no arguments, calls nothing and has no frame.
 * @source 0x801D2D74
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkBytes9ToBAndHalfword30ThenAdvanceStateByte1World00Area03004_801D2D74(void) {
  u8* work;
  u8* cur;

  work = D_1F800044;
  work[9] = 0;
  cur = D_1F800044;
  *(u16*)(work + 0x30) = 0;
  cur[0xA] = 0;
  D_1F800044[0xB] = 0;
  D_1F800044[1]++;
}

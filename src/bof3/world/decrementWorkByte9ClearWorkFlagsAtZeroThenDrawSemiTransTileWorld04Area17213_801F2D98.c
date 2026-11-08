#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

void func_80196070(void);
void func_801F2DF4(s32 arg0);

/* @source 0x801F2D98
 * @behavior State handler listed as entry 4 of this overlay handler table
 *           0x801F507C, which the dispatcher 0x801F2C04 indexes with work
 *           record byte +1. Decrements the countdown byte at offset 0x09 of the
 *           work record published at scratchpad pointer cell 0x1F800044 and,
 *           once the reloaded countdown reads zero, resets the shared work
 *           area through the exe-resident helper 0x80196070. Both paths then
 *           emit the overlay semi-transparent full-screen tile through the
 *           overlay helper 0x801F2DF4 with blend-rate argument 1. Takes no
 *           arguments and returns nothing; the record pointer is reloaded for
 *           the countdown test.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ClearWorkFlagsAtZeroThenDrawSemiTransTileWorld04Area17213_801F2D98(
    void) {
  D_1F800044[9]--;
  if (D_1F800044[9] == 0) {
    func_80196070();
  }
  func_801F2DF4(1);
}

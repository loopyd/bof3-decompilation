#include "bof3/bof3.h"

void func_800AEEC8(u8, u8);

/* Scratchpad work-record pointer cell. The original materialises this cell with
 * a constant-address offset load at each of its three uses (before the call,
 * then once per later store), so the source keeps the named cell form and
 * evaluates it at every use.
 * @source 0x1F800044 @kind data
 */
extern u8 *D_1F800044;

/* @source 0x801EF8FC
 * @behavior First step handler of this overlay's two-entry work state-byte-2
 *           handler table at 0x801EF9D0 (its first word; 0x801EF8B8 indexes
 *           that table with the work record's byte 2). Reads the scratchpad
 *           work record through the pointer cell 0x1F800044 once and passes
 *           its unsigned byte 4 as the first argument and its unsigned byte 3
 *           as the second to the battle helper at 0x800AEEC8, then stores 1 in
 *           the record's byte 0xB and increments the record's unsigned state
 *           byte at offset 2, which advances the dispatcher to the second entry
 *           at 0x801EF954. Each of the three cell reads is its own
 *           constant-address offset load, as the original has it. Takes no
 *           arguments, returns nothing and reads or writes no other state; the
 *           0x18-byte frame only keeps $ra across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperWithWorkBytes4And3SetWorkByteBToOneAndIncrementWorkByte2BmagicMagic03803_801EF8FC(
    void) {
  func_800AEEC8(D_1F800044[4], D_1F800044[3]);
  D_1F800044[0xB] = 1;
  D_1F800044[2]++;
}

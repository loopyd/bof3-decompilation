#include "bof3/bof3.h"

void func_800AEEC8(u8, u8);

/* Scratchpad work-record pointer cell. The original materialises this cell with
 * a constant-address offset load at each of its uses (once for the two call
 * arguments, then once per later store), so the source keeps the named cell
 * form and evaluates it at every use.
 * @source 0x1F800044 @kind data
 */
extern u8 *g_battle_work;

/* @source 0x801F02E4
 * @behavior First step handler of this overlay's two-entry work state-byte-2
 *           handler table at 0x801F04E8, the word at 0x801F04E8 and the only
 *           word in the payload that holds 0x801F02E4. That table's second word
 *           0x801F04EC holds 0x801F033C, and the dispatcher at 0x801F02A0
 *           indexes the table with the work record's byte 2, so incrementing
 *           that byte is what advances dispatch to the second handler. Reads the
 *           scratchpad work object through the pointer cell at 0x1F800044 once
 *           and passes its unsigned byte 4 as the first argument and its
 *           unsigned byte 3 as the second to the shared helper at 0x800AEEC8,
 *           then stores 1 in the record's byte 0xB and increments the record's
 *           unsigned byte at offset 2, reloading the cell for each store. The
 *           cell load precedes frame creation because the arguments need no
 *           stack; the 0x18-byte frame only keeps $ra at 0x10 across the call.
 *           Takes no arguments, returns nothing through the jr $ra at
 *           0x801F0334 and reads or writes no other state. The roles of work
 *           object bytes 4, 3, 0xB and 2 are not established, so they stay
 *           offset-anchored. The 88-byte span was byte-identical to the payload
 *           span of the exact lift
 *           invokeHelperWithWorkBytes4And3SetWorkByteBToOneAndIncrementWorkByte2BmagicMagic03803_801EF8FC,
 *           proven by one payload comparison in this lane; that lift supplied
 *           the statement order, operand forms and declaration style written
 *           here, and the own-target facts above differ.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callHelperOneIncrement(
    void) {
  func_800AEEC8(g_battle_work[4], g_battle_work[3]);
  g_battle_work[0xB] = 1;
  g_battle_work[2]++;
}

#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE058
 * @behavior Overlay handler reached only indirectly: the address is word 25
 * (the word stored at 0x801FE520) of the 38-entry in-image code-pointer run at
 * 0x801FE4BC that the target map names
 * coordinateLookupHandlers, which func_801FD280 (0x801FD280)
 * indexes with the signed byte returned by func_801A7AD8(0x801FE474, 0x0E) and
 * calls with jalr, so no in-image jal or j targets this address and that table
 * word is its only reference. It first re-arms the overlay effect group for the
 * ids 7, 4 and 8 through func_80166E88(7, 4, 8, 0) - a0=7, a1=4 and a2=8
 * materialised in that order before the jal and a3 zeroed by addu in that
 * call's delay slot - then calls the helper at 0x801C1400 with the constant
 * argument 0 (a0 zeroed by addu in that jal's delay slot), then the per-id
 * helper func_801C187C with the constant argument 4, the same per-id helper
 * again with the constant argument 8, and finally func_801C1630, whose delay
 * slot is a plain nop. It takes no arguments, returns nothing, reads no memory
 * of its own and writes none; the 0x18-byte frame only keeps $ra across the
 * five calls and each constant is materialised by its own addiu before its jal.
 * Its 76 bytes are the same instruction sequence as the gate-exact sibling
 * armEffect785Helper8Plus5 (0x801FDD7C,
 * table entry 16): a direct comparison of the two 76-byte payload spans showed
 * four differing bytes, the func_80166E88 second and third arguments and the
 * two func_801C187C arguments, so that sibling supplied shape only and not a
 * byte-identical clone, and the five neighbouring arm-and-helpers spans of the
 * same run (0x801FDD7C, 0x801FDE80, 0x801FE014, 0x801FE0A4, 0x801FE0E8) are
 * each unequal to these bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffect748Helper4Plus8(void) {
  func_80166E88(7, 4, 8, 0);
  func_801C1400(0);
  func_801C187C(4);
  func_801C187C(8);
  func_801C1630();
}

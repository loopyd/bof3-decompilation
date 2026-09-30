#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FDECC
 * @behavior Overlay handler reached only indirectly: the address is word 21
 * (the word stored at 0x801FE510) of the 38-entry in-image code-pointer table
 * at 0x801FE4BC that func_801FD280 (0x801FD280) indexes with the signed byte
 * returned by func_801A7AD8(0x801FE474, 0x0E) and calls with jalr, so no
 * in-image jal targets this address. It first re-arms the overlay effect group
 * for the ids 7, 6 and 5 through func_80166E88(7, 6, 5, 0) - a0=7, a1=6, a2=5
 * and a3 zeroed in that call's delay slot - then calls the helper at 0x801C1400
 * with the constant argument 0, then the per-id helper func_801C187C with the
 * constant argument 6, and finally func_801C1630 with no arguments. It takes no
 * arguments of its own, reads no memory and writes none; the 0x18-byte frame
 * only keeps $ra across the four calls, each constant is materialised by its own
 * addiu before its jal and the func_801C1630 call keeps a nop slot. Its 68 bytes
 * are the same arm-then-three-helpers shape as the gate-exact family siblings in
 * this overlay (0x801FDCF4, 0x801FDD38, 0x801FDDC8, 0x801FDE0C and 0x801FDE50 of
 * the same table) and differ from func_801FDF10 (0x801FDF10, the next table
 * word) in exactly one byte, the immediate of the func_801C187C argument (6 here,
 * 5 there), and from
 * armEffectGroupThenInvokeThreeHelpersScenarioScena0900_801FDE0C (0x801FDE0C,
 * index 18 of the same table) in exactly two bytes, the func_80166E88 second
 * argument (6 here, 8 there) and that same func_801C187C argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeThreeHelpersScenarioScena0900_801FDECC(void) {
  func_80166E88(7, 6, 5, 0);
  func_801C1400(0);
  func_801C187C(6);
  func_801C1630();
}

#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FDF10
 * @behavior Overlay handler reached only indirectly: the address is word 22
 * (the word stored at 0x801FE514) of the 38-entry in-image code-pointer table
 * at 0x801FE4BC that func_801FD280 (0x801FD280) indexes with the signed byte
 * returned by func_801A7AD8(0x801FE474, 0x0E) and calls with jalr, so no
 * in-image jal targets this address. It first re-arms the overlay effect group
 * for the ids 7, 6 and 5 through func_80166E88(7, 6, 5, 0) - a0=7, a1=6, a2=5
 * and a3 zeroed in that call's delay slot - then calls the helper at 0x801C1400
 * with the constant argument 0, then the per-id helper func_801C187C with the
 * constant argument 5, and finally func_801C1630 with no arguments. It takes no
 * arguments of its own, reads no memory and writes none; the 0x18-byte frame
 * only keeps $ra across the four calls, each constant is materialised by its own
 * addiu before its jal and the func_801C1630 call keeps a nop slot. Its 68 bytes
 * differ from the gate-exact family sibling
 * armEffectGroupThenInvokeThreeHelpersScenarioScena0900_801FDECC (0x801FDECC,
 * the preceding table word) in exactly one byte, the immediate of the
 * func_801C187C argument (5 here, 6 there), and its remaining 67 bytes are
 * byte-identical to that sibling; it differs from
 * armEffectGroupThenInvokeThreeHelpersScenarioScena0900_801FDDC8 (0x801FDDC8,
 * index 17 of the same table) in exactly one byte, the func_80166E88 second
 * argument (6 here, 8 there).
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeThreeHelpersScenarioScena0900_801FDF10(void) {
  func_80166E88(7, 6, 5, 0);
  func_801C1400(0);
  func_801C187C(5);
  func_801C1630();
}

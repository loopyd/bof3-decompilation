#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE0A4
 * @behavior Overlay handler reached only indirectly: the address is word 26
 * (the word stored at 0x801FE524) of the 38-entry in-image code-pointer table
 * at 0x801FE4BC that func_801FD280 (0x801FD280) indexes with the signed byte
 * returned by func_801A7AD8(0x801FE474, 0x0E) and calls with jalr, so no
 * in-image jal targets this address and that table word is its only reference.
 * It first re-arms the three-entry overlay effect group for the ids 7, 4 and 8
 * through func_80166E88(7, 4, 8, 0) - a0=7, a1=4 and a2=8 materialised in that
 * order before the jal and a3 zeroed in that call's delay slot - then calls the
 * helper at 0x801C1400 with the constant argument 0, then the per-id helper
 * func_801C187C with the constant argument 4, and finally func_801C1630 with no
 * arguments. It takes no arguments of its own, reads no memory and writes none;
 * the 0x18-byte frame only keeps $ra across the four calls, each constant is
 * materialised by its own addiu before its jal - the 0 for func_801C1400 in
 * that jal's delay slot - and the func_801C1630 call keeps a nop slot. Its 68
 * bytes differ from the gate-exact family sibling
 * armEffectGroupThenInvokeThreeHelpersScenarioScena0900_801FE014 (0x801FE014,
 * table entry 24) in exactly one byte, the immediate of the func_801C187C
 * argument (4 here, 8 there); the other 67 bytes are byte-identical to it, so
 * this is shape evidence and not a byte-identical clone. No byte-identical
 * sibling exists in this repository's lifted sources.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeThreeHelpersScenarioScena0900_801FE0A4(void) {
  func_80166E88(7, 4, 8, 0);
  func_801C1400(0);
  func_801C187C(4);
  func_801C1630();
}

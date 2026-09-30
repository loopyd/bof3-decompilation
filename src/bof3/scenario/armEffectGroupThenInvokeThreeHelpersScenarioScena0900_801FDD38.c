#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FDD38
 * @behavior Overlay handler reached only indirectly: the address is word 15
 * (the word stored at 0x801FE4F8) of the 38-entry in-image code-pointer table
 * at 0x801FE4BC that func_801FD280 (0x801FD280) indexes with the signed byte
 * returned by func_801A7AD8(0x801FE474, 0x0E) and calls with jalr, so no
 * in-image jal targets this address. It first re-arms the overlay effect group
 * for the ids 7, 8 and 2 through func_80166E88(7, 8, 2, 0) - a0=7, a1=8, a2=2
 * and a3 zeroed in that call's delay slot - then calls the helper at 0x801C1400
 * with the constant argument 0, then the per-id helper func_801C187C with the
 * constant argument 2, and finally func_801C1630 with no arguments. It takes no
 * arguments of its own, reads no memory and writes none; the 0x18-byte frame
 * only keeps $ra across the four calls. The 68 bytes are byte-identical to
 * func_801FDCF4 (0x801FDCF4, the preceding table word) except for the immediate
 * of the func_801C187C argument (2 here, 8 there), and they differ from the
 * exact sibling armEffectGroupThenInvokeThreeHelpersScenarioScena1100_801FABC0
 * (0x801FABC0 of emi/scenario/scena11/00, compared directly in both shipped
 * payloads) in exactly two bytes, the func_80166E88 third argument (2 here, 5
 * there) and that same func_801C187C argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeThreeHelpersScenarioScena0900_801FDD38(void) {
  func_80166E88(7, 8, 2, 0);
  func_801C1400(0);
  func_801C187C(2);
  func_801C1630();
}

#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE048
 * @behavior Overlay handler reached only indirectly (no in-image `jal` targets
 * it): it first re-arms the overlay effect group for the ids 0, 1 and 5
 * through func_80166E88(0, 1, 5, 0) - a0=0, a1=1 and a2=5 materialised before
 * the jal and a3 zeroed in that call's delay slot - then calls the helper at
 * 0x801C1400 with the constant argument 0, then the per-id helper func_801C187C
 * with the constant argument 5, and finally func_801C1630 with no arguments. It
 * takes no arguments of its own, reads no memory and writes none; the 0x18-byte
 * frame only keeps $ra across the four calls, each constant is materialised by
 * its own instruction before its jal - the 0 for func_801C1400 in that jal's
 * delay slot and the 5 in the delay slot of the func_801C187C jal - and the
 * func_801C1630 call keeps a nop slot. Its only image word is the pointer at
 * 0x801FE4A0, 0-based word 5 of the 22-entry in-image code-pointer list at
 * 0x801FE48C (0x801FDD7C ... 0x801FE38C, terminated by the null word at
 * 0x801FE4E4) that func_801FDD14 selects from with the signed index
 * func_801A7AD8(0x801FE478, 4) returns. The 68 bytes are byte-identical to the
 * exact sibling armEffectGroupThenInvokeThreeHelpersScenarioScena0900_801FDCF4
 * (0x801FDCF4 of emi/scenario/scena09/00) except for the three func_80166E88
 * ids, which are 7, 8 and 2 there and 0, 1 and 5 here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeThreeHelpersScenarioScena0600_801FE048(void) {
  func_80166E88(0, 1, 5, 0);
  func_801C1400(0);
  func_801C187C(5);
  func_801C1630();
}

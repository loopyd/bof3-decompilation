#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FDD7C
 * @behavior Overlay handler reached only indirectly: the address is word 16
 * (the word stored at 0x801FE4FC) of the 38-entry in-image code-pointer run at
 * 0x801FE4BC that func_801FD280 (0x801FD280) indexes with the signed byte
 * returned by func_801A7AD8(0x801FE474, 0x0E) and calls with jalr, so no
 * in-image jal or j targets this address. It first re-arms the overlay effect
 * group for the ids 7, 8 and 5 through func_80166E88(7, 8, 5, 0) - a0=7, a1=8
 * and a2=5 materialised before the jal and a3 zeroed by addu in that call's
 * delay slot - then calls the helper at 0x801C1400 with the constant argument
 * 0 (a0 zeroed in its delay slot), then the per-id helper func_801C187C with
 * the constant argument 8, then the same per-id helper again with the constant
 * argument 5, and finally func_801C1630 (its delay slot is a plain nop). It
 * takes no arguments, returns nothing, reads no memory of its own and writes
 * none; the 0x18-byte frame only keeps $ra across the five calls and each
 * later constant is materialised in the delay slot of its own call. The 76
 * bytes were byte-identical to the then-exact sibling
 * armEffectGroupThenInvokeFourHelpersScenarioScena1100_801FAB74 (0x801FAB74 of
 * emi/scenario/scena11/00, compared directly in both shipped payloads), and
 * they are the three-helper arm-and-helper tail of the neighbouring table
 * words 0x801FDD38 and 0x801FDDC8 with one extra func_801C187C(5) call
 * inserted between the func_801C187C(8) call and the func_801C1630 tail.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeFourHelpersScenarioScena0900_801FDD7C(void) {
  func_80166E88(7, 8, 5, 0);
  func_801C1400(0);
  func_801C187C(8);
  func_801C187C(5);
  func_801C1630();
}

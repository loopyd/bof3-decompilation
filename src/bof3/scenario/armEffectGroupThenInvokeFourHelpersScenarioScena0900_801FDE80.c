#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FDE80
 * @behavior Overlay handler reached only indirectly: the address is word 20
 * (the word stored at 0x801FE50C) of the 38-entry in-image code-pointer run at
 * 0x801FE4BC that func_801FD280 (0x801FD280) indexes with the signed byte
 * returned by func_801A7AD8(0x801FE474, 0x0E) and calls with jalr, so no
 * in-image jal or j targets this address. It first re-arms the overlay effect
 * group for the ids 7, 6 and 5 through func_80166E88(7, 6, 5, 0) - a0=7, a1=6
 * and a2=5 materialised before the jal and a3 zeroed by addu in that call's
 * delay slot - then calls the helper at 0x801C1400 with the constant argument
 * 0 (a0 zeroed in its delay slot), then the per-id helper func_801C187C with
 * the constant argument 5, then the same per-id helper again with the constant
 * argument 6, and finally func_801C1630 (its delay slot is a plain nop). It
 * takes no arguments, returns nothing, reads no memory of its own and writes
 * none; the 0x18-byte frame only keeps $ra across the five calls and each
 * later constant is materialised in the delay slot of its own call. The 76
 * bytes are the same arm-and-four-helpers shape as the gate-exact sibling
 * armEffectGroupThenInvokeFourHelpersScenarioScena0900_801FDD7C (0x801FDD7C,
 * word 16 of the same table), which differs only in this function's
 * func_80166E88 second argument (6 here, 8 there) and in the two
 * func_801C187C immediates (5 then 6 here, 8 then 5 there). The same 76 bytes
 * also appear verbatim at 0x801FAD1C of emi/scenario/scena11/00 and at
 * 0x801FCF88 of emi/scenario/scena12/00, which share this load address but
 * still hold that entry as raw 'asm', so those twins corroborate the shape
 * without being a lifted clone source.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeFourHelpersScenarioScena0900_801FDE80(void) {
  func_80166E88(7, 6, 5, 0);
  func_801C1400(0);
  func_801C187C(5);
  func_801C187C(6);
  func_801C1630();
}

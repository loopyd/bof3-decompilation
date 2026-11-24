#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FCFD4
 * @behavior Overlay per-record callback and entry 11, the word at 0x801FD268,
 * of the nineteen-word code-pointer run based at 0x801FD23C that the selector
 * callCoordinateRecordSelect (0x801FCC54)
 * indexes with the signed byte func_801A7AD8 returns for the four five-byte
 * lookup records at 0x801FD228, whose records select only indices 0 through 3;
 * that run word is the only reference to this address and no jal or j targets
 * it. It first re-arms the three-entry overlay effect group for ids 7, 6 and 5
 * through func_80166E88(7, 6, 5, 0), then calls the helper at 0x801C1400 with
 * the constant argument 0, then the per-id teardown helper func_801C187C with
 * the constant argument 6, and finally func_801C1630. It takes no arguments,
 * reads no memory and writes none; the 0x18-byte frame only keeps $ra across
 * the four calls, each argument constant is materialised by its own addiu
 * before its jal and the func_801C1630 call keeps a nop slot. Its 68 bytes are
 * byte-identical to the exact siblings
 * armEffectGroupThenInvokeThreeHelpersScenarioScena1100_801FAD68 at 0x801FAD68
 * of emi/scenario/scena11/00 and
 * armEffectGroupThenInvokeThreeHelpersScenarioScena0900_801FDECC at 0x801FDECC
 * of emi/scenario/scena09/00, compared in the shipped payloads at their own
 * load addresses; this target's neighbours 0x801FCE70 and 0x801FCF44 carry the
 * same four calls in the same order and differ only in the ids the arm call
 * passes and in the func_801C187C argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffect765Helpers(void) {
  func_80166E88(7, 6, 5, 0);
  func_801C1400(0);
  func_801C187C(6);
  func_801C1630();
}

#include "bof3/bof3.h"

extern u8 D_80144EA2;
extern u8 D_80146875;
extern u16 D_80146876;

void func_8015C088(void);

/* @source 0x801FC5A4
 * @behavior Guarded state request of this overlay: it is not an entry of the
 * overlay's nine-pointer record callback table at 0x801FD00C, but the shared
 * tail of that table's four scenario-flag slots
 * setScenarioFlagBit10ThenInvokeHelperScenarioScena0300_801FC4F4,
 * setScenarioFlagBit11ThenInvokeHelperScenarioScena0300_801FC520,
 * setScenarioFlagBit12ThenInvokeHelperScenarioScena0300_801FC54C and
 * setScenarioFlagBit13ThenInvokeHelperScenarioScena0300_801FC578, which each
 * raise one bit of the shared scenario flag word and then call it without
 * arguments, discarding its result. It reads the shared main-RAM byte
 * D_80144EA2 (0x80144EA2) with lui/lbu and masks its low nibble, and when that
 * nibble is not 0xF it answers 0 without touching any other state, jumping
 * straight to the shared epilogue with move $v0,$zero in the jump delay slot.
 * When the nibble is 0xF it runs the shared front-end startup helper
 * func_8015C088 (0x8015C088), requests secondary sub-state 6 by storing 6 in
 * the shared sub-state byte D_80146875 and arms the shared unsigned halfword
 * countdown D_80146876 with 0x1E (30), then answers 1; the shared primary state
 * byte D_80146874 is left unchanged, unlike its table neighbours
 * func_801FC690 and requestPrimaryState7AndArmCountdown30ScenarioScena0300_801FC6C4,
 * which request a primary state of their own. The 0x18-byte frame only keeps
 * $ra across the call, each of the two stores materialises the 0x8014 page in
 * $at on its own, and both returned constants are materialised in $v0 by li.
 * The guard is written as the guarded work arm with the plain `return 0` after
 * it, which is the form that emits beq to the out-of-line work arm and the
 * `j <epilogue>` / move $v0,$zero pair on the fallthrough; the inverted early
 * return of 0 in front of the block instead makes the psyq compiler place that
 * single return out of line (measured 18/22 instructions, same 88 bytes, first
 * difference at the branch: bne v0,v1 to a trailing move $v0,$zero). The exact
 * scena09 twin
 * waitFlag2CThenRequestPrimaryState9Substate14ScenarioScena0900_801FDC1C
 * records that a single result local carried across the calls costs a saved
 * register and frame bytes (shape evidence only, no byte identity).
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 requestSubState6AndArmCountdown30WhenSharedNibbleIs15ScenarioScena0300_801FC5A4(void) {
  if ((D_80144EA2 & 0xF) == 0xF) {
    func_8015C088();
    D_80146875 = 6;
    D_80146876 = 0x1E;
    return 1;
  }

  return 0;
}

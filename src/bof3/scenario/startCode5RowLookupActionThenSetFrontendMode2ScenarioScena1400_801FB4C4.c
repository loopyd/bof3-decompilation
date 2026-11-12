#include "bof3/bof3.h"

extern u16 D_80143F00; /* @source 0x80143F00 @kind data */
extern u8 D_80143BB0;  /* @source 0x80143BB0 @kind data */
extern u16 func_801F326C(u32 arg0);       /* @source 0x801F326C */
extern u16 func_801F37AC(u32 arg0);       /* @source 0x801F37AC */
extern void func_80150224(s32 action_id); /* @source 0x80150224 */

/* @source 0x801FB4C4
 * @behavior Overlay per-record callback: it reads the shared 0x80143F00
 * halfword and compares it with 0xC0, then takes the code-5 row answer from
 * func_801F326C when the two are equal and from func_801F37AC otherwise. That
 * 16-bit answer is sign-extended into the argument of the shared action
 * requester func_80150224, and the body finally sets the shared frontend mode
 * byte at 0x80143BB0 to 2. It ignores the record pointer and second argument
 * its dispatcher passes, returns nothing, and its 0x18-byte frame only keeps
 * $ra across the call. The two answer paths each end with the `sll $v0,$v0,16`
 * of the argument conversion and the single `sra $a0,$v0,16` sits in the
 * requester's delay slot. No in-image `jal` targets the address and no other
 * word of the payload holds it; its only image word is 0x801FC474, entry 3 of
 * the twenty-two-word callback run at 0x801FC468 that
 * dispatchRecordCallbackByByte7AScenarioScena1400_801FB37C (0x801FB37C)
 * selects with the unsigned record byte at offset 0x7A. The body equals the
 * ones at 0x801FB3BC and 0x801FB46C word for word except the two lookup
 * immediates here, which are 5 where those spans pass 8 and 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startCode5RowLookupActionThenSetFrontendMode2ScenarioScena1400_801FB4C4(
    void) {
  func_80150224(
      (s16)(D_80143F00 == 0xC0 ? func_801F326C(5) : func_801F37AC(5)));
  D_80143BB0 = 2;
}

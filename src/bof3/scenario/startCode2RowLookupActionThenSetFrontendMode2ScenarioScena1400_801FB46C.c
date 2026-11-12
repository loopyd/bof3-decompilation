#include "bof3/bof3.h"

extern u16 D_80143F00; /* @source 0x80143F00 @kind data */
extern u8 D_80143BB0;  /* @source 0x80143BB0 @kind data */
extern u16 func_801F326C(u32 arg0);       /* @source 0x801F326C */
extern u16 func_801F37AC(u32 arg0);       /* @source 0x801F37AC */
extern void func_80150224(s32 action_id); /* @source 0x80150224 */

/* @source 0x801FB46C
 * @behavior Overlay per-record callback at 0x801FB46C: it reads the shared
 * halfword at 0x80143F00 and compares it with 0xC0, then takes the code-2 row
 * answer from func_801F326C when the two are equal and from func_801F37AC
 * otherwise, passing 2 as the lookup code on both paths. That 16-bit answer is
 * sign-extended into the single argument of the shared action requester
 * func_80150224, and the body finally stores 2 in the shared frontend mode
 * byte at 0x80143BB0. It ignores the record pointer and the second argument
 * its dispatcher passes, returns nothing, and its 0x18-byte frame only keeps
 * $ra across the call. Both answer paths end with the sll of the argument
 * conversion and the one sra sits in the requester delay slot. No in-image jal
 * targets the address; its only image word is 0x801FC470, entry 2 of the
 * twenty-two-word callback run at 0x801FC468 that
 * dispatchRecordCallbackByByte7AScenarioScena1400_801FB37C (0x801FB37C)
 * selects with the unsigned record byte at offset 0x7A. The body equals the
 * one at 0x801FB3BC word for word except the two lookup immediates here, which
 * are 2 where that span passes 8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startCode2RowLookupActionThenSetFrontendMode2ScenarioScena1400_801FB46C(
    void) {
  func_80150224(
      (s16)(D_80143F00 == 0xC0 ? func_801F326C(2) : func_801F37AC(2)));
  D_80143BB0 = 2;
}

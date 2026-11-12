#include "bof3/bof3.h"

extern u16 D_80143F00; /* @source 0x80143F00 @kind data */
extern u8 D_80143BB0;  /* @source 0x80143BB0 @kind data */
extern u16 func_801F326C(u32 arg0);       /* @source 0x801F326C */
extern u16 func_801F37AC(u32 arg0);       /* @source 0x801F37AC */
extern void func_80150224(s32 action_id); /* @source 0x80150224 */

/* @source 0x801FB51C
 * @behavior Overlay per-record callback (entry 4 of the twenty-two-word
 * callback run at 0x801FC468, which
 * dispatchRecordCallbackByByte7AScenarioScena1400_801FB37C (0x801FB37C)
 * selects with the unsigned record byte at offset 0x7A; the word at 0x801FC478
 * holds this address, its only image word). It reads the shared halfword
 * D_80143F00, compares it with 0xC0, and takes the code-6 row answer from
 * func_801F326C when the two are equal and from func_801F37AC otherwise,
 * passing 6 as the lookup code on both paths; the 16-bit answer is
 * sign-extended into the argument of the shared action requester func_80150224,
 * and the body finally stores 2 in the shared frontend mode byte D_80143BB0.
 * It ignores the record pointer and second argument its dispatcher passes,
 * reads no memory besides D_80143F00, writes only D_80143BB0, and its
 * 0x18-byte frame only keeps $ra across the call, with the frame store in the
 * branch delay slot and each answer path ending in the sll of the argument
 * conversion while the sra uses the requester delay slot. No in-image jal
 * targets the address. A payload comparison of the two 0x58-byte spans showed
 * this body equal to the one at 0x801FB4C4 in every word but these two lookup
 * immediates, which are 6 where that span passed 5, and that sibling's own
 * local jump word, which shifts by the 0x58 span delta.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startCode6RowLookupActionThenSetFrontendMode2ScenarioScena1400_801FB51C(
    void) {
  func_80150224(
      (s16)(D_80143F00 == 0xC0 ? func_801F326C(6) : func_801F37AC(6)));
  D_80143BB0 = 2;
}

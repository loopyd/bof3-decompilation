#include "bof3/bof3.h"

/* @source 0x8014686C @kind unknown */
extern u32 D_8014686C;
/* @source 0x80146874 @kind unknown */
extern u8 D_80146874;
/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;
/* @source 0x80145F09 @kind unknown — party byte (+0x79) of the 0x140-byte
 * stride local work records based at 0x80145E90, so this reads record 0. */
extern u8 D_80145F09;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015C088(void);

/* @source 0x801FD2E8
 * @behavior Overlay handler reached only indirectly: the address is word 0
 * (the word stored at 0x801FE4BC) of the 38-entry in-image code-pointer table
 * at 0x801FE4BC that dispatchCoordinateLookupHandlerScenarioScena0900_801FD280
 * (0x801FD280) indexes with the signed byte returned by func_801A7AD8
 * (0x801FE474, 0x0E) and calls with jalr, and that dispatcher returns this
 * handler's signed low byte, so no in-image jal or image data word of this
 * payload addresses 0x801FD2E8. It branches on the party byte of local work
 * record 0 (D_80145F09, 0x80145F09, read with lui/lbu): when that byte holds 5
 * it takes the two-stage wait func_8015B5D4 on the shared flag word D_8014686C
 * (0x8014686C, read with lui/lw into $a0) for wait id 0x0C. A zero return means
 * the wait is done, so it runs the shared front-end startup helper
 * func_8015C088 and arms sub-state 0x14 in the shared sub-state byte
 * D_80146875; a non-zero return means the wait is still running, so it runs
 * func_8015C088 and arms sub-state 5 instead. When the party byte holds
 * anything other than 5, no wait is taken: it runs func_8015C088 and arms
 * sub-state 0. All three paths then store 6 in the shared state byte
 * D_80146874, so primary scene state 6 is requested unconditionally and only
 * the sub-state differs, and all three answer 1, materialised as
 * addiu $v0,$zero,1 directly after the startup call. It takes no arguments,
 * reads no state beyond that one party byte and the wait word, and its
 * 0x18-byte frame only keeps $ra across the calls; the $ra save fills the bne
 * delay slot and each byte store materialises the 0x8014 page in $at on its
 * own. A bounded comparison of this 0xA0-byte span against every shipped EMI
 * payload under out/binaries/emi found no byte-identical copy, so no clone
 * candidate exists; the neighbouring table words 1 to 5 (0x801FD388 to
 * 0x801FD7F4) and word 6
 * (waitFlag3DThenRequestPrimaryState8Substate0AndSetFlagBit18OrSubstate5ScenarioScena0900_801FD8F0,
 * 0x801FD8F0) keep only the wait-then-request shape, which is shape evidence
 * and not clone proof.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* Matching note: the original branches with bne to the party-word arm, so the
 * party test is the positive `== 5` guard whose then-block falls through into
 * the wait and whose trailing block is the "anything other than 5" arm, and the
 * wait word is stored once at the shared tail. Writing the party arm as the
 * leading `if (D_80145F09 != 5) { ...; return 1; }` guard instead inverted the
 * test (beq to that arm) and put the wait first (measured 23/40 instructions,
 * 148 bytes, first difference +0x10). The same inverted party guard with the
 * wait arm written as a `!= 0` early return measured 37/40 instructions at the
 * full 160 bytes, first difference +0x28. Both the nested if/else form and this
 * trailing-block form then matched all 40 instructions and all 160 bytes.
 */
s32 waitFlag0CForParty5ThenRequestPrimaryState6Substate0Or14Or5ScenarioScena0900_801FD2E8(void) {
  if (D_80145F09 == 5) {
    if (func_8015B5D4(D_8014686C, 0x0C) == 0) {
      func_8015C088();
      D_80146875 = 0x14;
      D_80146874 = 6;
      return 1;
    }
    func_8015C088();
    D_80146875 = 5;
    D_80146874 = 6;
    return 1;
  }
  func_8015C088();
  D_80146875 = 0;
  D_80146874 = 6;
  return 1;
}

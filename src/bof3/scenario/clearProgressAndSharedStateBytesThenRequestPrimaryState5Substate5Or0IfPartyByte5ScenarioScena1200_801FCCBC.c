#include "bof3/bof3.h"

/* @source 0x80145F09 @kind unknown - party byte (+0x79) of the 0x140-byte
 * stride local work records based at 0x80145E90, so this reads record 0. */
extern u8 D_80145F09;
extern u8 g_ScenarioProgress;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FCCBC
 * @behavior Overlay handler reached only indirectly: the three words at
 * 0x801FD23C, entries 0, 1 and 2 of the nineteen-word code-pointer run that the
 * coordinate selector dispatchCoordinateLookupHandlerScenarioScena1200_801FCC54
 * (0x801FCC54) indexes with the signed byte func_801A7AD8 returns and calls
 * with jalr, so three of the four selectable lookup records run this body and
 * that selector answers its signed low byte. No in-image jal and no other image
 * word names this address. It runs the shared front-end startup helper
 * func_8015C088 (0x8015C088), clears the shared progress byte g_ScenarioProgress
 * (0x80146864) and the shared state bytes 0x80146865, 0x80146866 and 0x80146867,
 * then requests primary scene state 5 in the shared state byte D_80146874 and
 * selects the secondary sub-state from the party byte of local work record 0
 * (D_80145F09, 0x80145F09, read with lui/lbu): sub-state 0 in D_80146875 when
 * that byte holds anything other than 5, sub-state 5 when it holds 5. It
 * answers 1 on both paths. It takes no arguments, ignores the record pointer and
 * second argument its dispatcher passes, reads no state beyond that one party
 * byte and writes nothing beyond those six bytes. The 0x18-byte frame only keeps
 * $ra across the call, each byte store materialises the 0x8014 page in $at on
 * its own, and the store order g_ScenarioProgress, 0x80146865, 0x80146866,
 * 0x80146867, D_80146875, D_80146874 is the source statement order. The psyq
 * compiler materialises the compared constant 5 in $v0 ahead of the four
 * clearing stores, the stored 5 in $v1 inside each arm of the party test, and
 * the answer 1 in $v0, which the bne delay slot then carries. Its 0x7C bytes
 * keep the startup-then-clear-progress-and-request-state-5 shape of the exact
 * siblings
 * setSubstate2ClearProgressAndSharedStateBytesAndRequestPrimaryState5ScenarioScena1200_801FC6E0
 * (0x801FC6E0) and
 * setSubstate1EClearSharedStateBytesAndRequestPrimaryState8ScenarioScena1200_801FC784
 * (0x801FC784) of this overlay, and the party-test-then-request shape with the
 * answer 1 of
 * waitFlag0CForParty5ThenRequestPrimaryState6Substate0Or14Or5ScenarioScena0900_801FD2E8
 * (0x801FD2E8 of emi/scenario/scena09/00); a bounded comparison of this span
 * against every shipped EMI payload under out/binaries/emi found no
 * byte-identical copy, so those three are shape evidence and not clone proof.
 * The name records the proven behavior and the address anchor keeps it
 * target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* Matching note: the original branches with bne to the sub-state-0 arm, so the
 * party test is written as the leading `if (D_80145F09 != 5)` guard that the
 * branch reaches, with the sub-state-5 arm as the trailing block that falls
 * through into the shared `D_80146874 = 5` store. Seeding the same stores with a
 * single `return 1` after an if/else party test measured 23/31 instructions
 * (124 bytes, first difference +0x40 = the bne delay slot, where the original
 * carries `li v0,1` and that seed carried nop), and moved the answer into $v0 at
 * the tail so the stored 5 took $v0 instead of $v1. Giving each arm its own
 * `return 1` hoisted the answer into the delay slot but inverted the branch
 * (beq, measured 25/31 instructions, 120 bytes, first difference +0x3C); the
 * guard form above matched all 31 instructions and all 124 bytes.
 */
s32 clearProgressAndSharedStateBytesThenRequestPrimaryState5Substate5Or0IfPartyByte5ScenarioScena1200_801FCCBC(
    void) {
  func_8015C088();
  g_ScenarioProgress = 0;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  if (D_80145F09 != 5) {
    D_80146875 = 0;
    D_80146874 = 5;
    return 1;
  }
  D_80146875 = 5;
  D_80146874 = 5;
  return 1;
}

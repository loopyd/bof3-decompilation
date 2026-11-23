#include "bof3/scenario/scena09_state.h"

/* @source 0x8014686C @kind unknown */
extern u32 D_8014686C;
/* @source 0x80145F09 @kind unknown - party byte (+0x79) of the 0x140-byte
 * stride local work records based at 0x80145E90, so this reads record 0. */
extern u8 D_80145F09;
/* @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;

s32 func_8015B5D4(u32 arg0, s32 arg1);

/* @source 0x801FD5A8
 * @behavior Overlay handler reached only indirectly: the address is word 3
 * (the word stored at 0x801FE4C8) of the 38-entry in-image code-pointer table
 * at 0x801FE4BC that callCoordinateLookupScenario
 * (0x801FD280) indexes with the signed byte returned by func_801A7AD8
 * (0x801FE474, 0x0E) and calls with jalr, so no in-image jal targets this
 * address and that one table word is its only reference in this payload. It
 * first reads the party byte of local work record 0 (D_80145F09, 0x80145F09,
 * read with lui/lbu). While that byte holds 5 it walks a four-step chain of the
 * two-stage wait func_8015B5D4 on the shared flag word D_8014686C (0x8014686C,
 * reloaded with lui/lw into $a0 before each probe) for wait ids 0x10, then
 * 0x12, then 0x1D, then 0x11. The first wait that reports zero means that wait
 * is done, so it runs the shared front-end startup helper func_8015C088, arms
 * the shared sub-state byte D_80146875 with 2, 3, 0x32 or 7 for wait 0x10,
 * 0x12, 0x1D or 0x11 respectively and requests primary scene state 6 through
 * the shared state byte D_80146874; the 0x1D arm alone also clears the shared
 * scenario progress byte g_ScenarioProgress (0x80146864) and it clears it
 * before the state request. When all four waits are still running it instead
 * runs the same startup helper, arms sub-state 5 and requests that same primary
 * state 6. When the party byte holds anything other than 5 no wait is taken at
 * all: it runs func_8015C088, arms sub-state 0 and requests primary scene state
 * 6. Every arm answers 1, it takes no arguments, reads no state beyond that one
 * party byte and the wait word and writes no cell beyond the two shared state
 * bytes, the sub-state byte and the progress byte; its 0x18-byte frame only
 * keeps $ra across the calls, and the $ra save fills the bne delay slot. At the
 * time of writing this 0x150-byte span differed from the then exact sibling
 * waitFlag0EParty5
 * (0x801FD498) in the fourth wait step it adds, in the four wait ids (0x10,
 * 0x12, 0x1D, 0x11 here against 0x0E, 0x10, 0x0F there), in the third and
 * fourth sub-state constants (0x32 and 7 against 0x28 and 5), in the extra
 * progress-byte clear its 0x1D arm makes, and in the five internal jump words
 * retargeted to this body, while that sibling supplied the statement order,
 * operand forms, declaration style and the merged state store in the shared
 * tail. A scan of the 398 payloads under out/binaries found no byte-identical
 * copy of this span, not even with the jump, call and branch target fields
 * masked, so no clone candidate existed.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 waitRequestPrimaryProgress(
    void) {
  if (D_80145F09 == 5) {
    if (func_8015B5D4(D_8014686C, 0x10) == 0) {
      SCENA09_START_AND_REQUEST_STATE6(2);
      return 1;
    }
    if (func_8015B5D4(D_8014686C, 0x12) == 0) {
      SCENA09_START_AND_REQUEST_STATE6(3);
      return 1;
    }
    if (func_8015B5D4(D_8014686C, 0x1D) == 0) {
      func_8015C088();
      D_80146875 = 0x32;
      g_ScenarioProgress = 0;
      D_80146874 = 6;
      return 1;
    }
    if (func_8015B5D4(D_8014686C, 0x11) == 0) {
      SCENA09_START_AND_REQUEST_STATE6(7);
      return 1;
    }
    SCENA09_START_AND_REQUEST_STATE6(5);
    return 1;
  }
  SCENA09_START_AND_REQUEST_STATE6(0);
  return 1;
}

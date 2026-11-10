#include "bof3/bof3.h"

/* @source 0x8014686C @kind unknown */
extern u32 D_8014686C;
/* @source 0x80146874 @kind unknown */
extern u8 D_80146874;
/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;
/* @source 0x80145F09 @kind unknown - party byte (+0x79) of the 0x140-byte
 * stride local work records based at 0x80145E90, so this reads record 0. */
extern u8 D_80145F09;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015C088(void);

/* @source 0x801FD498
 * @behavior Overlay handler reached only indirectly: the address is word 2
 * (the word stored at 0x801FE4C4) of the 38-entry in-image code-pointer table
 * at 0x801FE4BC that dispatchCoordinateLookupHandlerScenarioScena0900_801FD280
 * (0x801FD280) indexes with the signed byte returned by func_801A7AD8
 * (0x801FE474, 0x0E) and calls with jalr, so no in-image jal addresses this
 * entry and the dispatcher returns this handler signed low byte. While the
 * party byte of local work record 0 (D_80145F09, 0x80145F09) holds 5 it walks
 * a three-step chain of the two-stage wait func_8015B5D4 on the shared flag
 * word D_8014686C (0x8014686C, reloaded with lui/lw into $a0 before each
 * probe) for wait ids 0x0E, then 0x10, then 0x0F. The first wait that reports
 * zero runs the shared front-end startup helper func_8015C088, arms the shared
 * sub-state byte D_80146875 with 2, 3 or 0x28 for wait 0x0E, 0x10 or 0x0F
 * respectively and requests primary scene state 6 through the shared state
 * byte D_80146874. When all three waits are still running it instead runs the
 * same startup helper, arms sub-state 5 and requests that same primary state
 * 6. When the party byte holds anything other than 5 no wait is taken at all:
 * it runs func_8015C088, arms sub-state 0 and requests primary scene state 6.
 * Every arm answers 1, it takes no arguments, reads no state beyond that one
 * party byte and the wait word, and its 0x18-byte frame only keeps $ra across
 * the calls; the $ra save fills the bne delay slot, each substate arm holds the
 * sub-state constant in $v1 while the state constant 6 fills its j delay slot,
 * and the other-party arm stores 0 through $zero with 6 already in $v1. At the
 * time of writing the 0x110-byte span differed from the then exact sibling
 * waitFlags0C0E0DForParty5ThenRequestPrimaryState6Substate2Or3Or1EOr5ScenarioScena0900_801FD388
 * (0x801FD388) in eight of its 68 original words only: that sibling supplied
 * the statement order and operand forms, while the three wait ids (0x0E, 0x10,
 * 0x0F here against 0x0C, 0x0E, 0x0D there), the third sub-state (0x28 against
 * 0x1E) and the four internal jumps retargeted to this address came from this
 * span own bytes. No shipped EMI payload under out/binaries held a
 * byte-identical copy of that span, so no clone candidate existed.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 waitFlags0E100FForParty5ThenRequestPrimaryState6Substate2Or3Or28Or5ScenarioScena0900_801FD498(void) {
  if (D_80145F09 == 5) {
    if (func_8015B5D4(D_8014686C, 0x0E) == 0) {
      func_8015C088();
      D_80146875 = 2;
      D_80146874 = 6;
      return 1;
    }
    if (func_8015B5D4(D_8014686C, 0x10) == 0) {
      func_8015C088();
      D_80146875 = 3;
      D_80146874 = 6;
      return 1;
    }
    if (func_8015B5D4(D_8014686C, 0x0F) == 0) {
      func_8015C088();
      D_80146875 = 0x28;
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

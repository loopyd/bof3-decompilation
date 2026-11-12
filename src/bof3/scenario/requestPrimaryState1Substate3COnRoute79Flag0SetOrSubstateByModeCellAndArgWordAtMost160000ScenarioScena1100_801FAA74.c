#include "bof3/bof3.h"

/* Shared route word of the main executable, tested here against 0x79; the
 * exact sibling requestPrimaryState2Substate3COnRoute79Flag0SetAndArgWordAtMost160000ScenarioScena1200_801FC80C
 * (0x801FC80C) tests the same halfword against the same route constant.
 * @source 0x80143F00 @kind data
 */
extern u16 D_80143F00;

/* Shared scenario flag word the main-executable flag tester func_8015B5D4
 * indexes for flag numbers.
 * @source 0x8014686C @kind unknown
 */
extern u32 D_8014686C;

/* Leading byte of the shared game-mode record table at 0x80145E98, the mode
 * value this body branches on; the generated undefined_syms_auto row of this
 * target already binds it because the still-assembled func_801F778C reads it.
 * @source 0x80145E98 @kind data
 */
extern u8 D_80145E98;

/* Shared primary state byte of the front-end state machine.
 * @source 0x80146874 @kind unknown
 */
extern u8 D_80146874;

/* Shared secondary sub-state byte of the front-end state machine.
 * @source 0x80146875 @kind unknown
 */
extern u8 D_80146875;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015C088(void);

/* @source 0x801FAA74
 * @behavior Overlay frame handler reached only indirectly: the address is word
 * 10 (0x801FAEE8) of the overlay-local handler run based at 0x801FAEC0 that
 * dispatchWorkByte1HandlerThenPublishMode2ScenarioScena1100_801F70B4
 * (0x801F70B4) indexes with the work record unsigned byte at offset 1 taken
 * through the scratchpad pointer cell 0x1F800044 and calls with jalr, so a work
 * byte reading 0x0A selects this body. That dispatcher writes only $v0 and $at
 * before the indirect call, so the word this body receives in $a0 is the
 * dispatching frame own leftover argument and the parameter stays structural;
 * no jal and no j anywhere in the payload reaches the address. The body answers
 * 0 unless the shared route word D_80143F00 reads 0x79, and then splits on the
 * main-executable flag tester func_8015B5D4 (0x8015B5D4) reading flag 0 of the
 * shared flag word D_8014686C. With flag 0 clear it first refuses an argument
 * word above 0x160000 under a signed comparison and then arms the shared
 * sub-state byte D_80146875 with 0 when the shared game-mode byte D_80145E98
 * reads 0, 6 or 7 and with 0x32 for every other mode; with flag 0 set it refuses
 * the same argument bound and arms D_80146875 with 0x3C. Every arm that acts
 * runs the shared front-end startup helper func_8015C088 (0x8015C088), requests
 * primary scene state 1 in the shared primary state byte D_80146874 and answers
 * 1; both refusals answer 0, the flag-clear one materialising that zero in the
 * delay slot of its own branch. The flag-clear arm is the then-block and the
 * flag-set arm the trailing else, which is the order the original shows: its
 * branch on the flag call leaves the game-mode walk in the fall-through and
 * lands the 0x3C arm after that walk. The argument bound is a separate source
 * test in each arm: the compiler hoisted the shared 0x160000 constant into the
 * delay slot of the flag-test branch and left one slt/bnez pair per arm. The
 * 0x18-byte frame keeps $ra and the argument in $s0 across the calls, each
 * shared byte store materialises the 0x8014 page in $at on its own, and the two
 * computed sub-state arms share one store tail because they differ only in the
 * constant they put in $v1. The leading twelve instructions are those of the
 * exact sibling requestPrimaryState2Substate3COnRoute79Flag0SetAndArgWordAtMost160000ScenarioScena1200_801FC80C
 * (0x801FC80C), which supplied the statement order, operand forms and
 * declaration style; that sibling inverts the flag test and stops at its
 * sub-state 0x3C arm with primary state 2, so the game-mode walk and every
 * stored value here come from this span own bytes. One comparison of the
 * 240-byte body against the payloads of every delivered binary found it only at
 * its own offset, so the clone lever had no candidate.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* Matching note: the measured shapes were the flag-set arm written first as an
 * early return (39/60 instructions at 228 bytes, first difference 0x18 on the
 * flag-test branch), the same with the game-mode test spelled
 * (u32)(D_80145E98 - 6) < 2 || D_80145E98 == 0 (39/60 at 228), the mode value
 * tested against 0 first (28/60 at 224) and a single guarded call run with the
 * sub-state picked into the branch (30/60 at 208). Writing the flag-clear arm as
 * the then-block and the flag-set arm as the trailing else matched 60/60 at the
 * original 240 bytes, so the arm order was the whole residual.
 */
s32 requestPrimaryState1Substate3COnRoute79Flag0SetOrSubstateByModeCellAndArgWordAtMost160000ScenarioScena1100_801FAA74(
    s32 arg0) {
  if (D_80143F00 != 0x79) {
    return 0;
  }

  if (func_8015B5D4(D_8014686C, 0) == 0) {
    if (arg0 > 0x160000) {
      return 0;
    }
    if (D_80145E98 == 6 || D_80145E98 == 7 || D_80145E98 == 0) {
      func_8015C088();
      D_80146875 = 0;
      D_80146874 = 1;
      return 1;
    }
    func_8015C088();
    D_80146875 = 0x32;
    D_80146874 = 1;
    return 1;
  }
  if (arg0 > 0x160000) {
    return 0;
  }
  func_8015C088();
  D_80146875 = 0x3C;
  D_80146874 = 1;
  return 1;
}

#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u16 D_801F535C[]; /* @source 0x801F535C @kind table */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */
extern u8 D_80146865; /* @source 0x80146865 @kind data */
extern u8 D_80146866; /* @source 0x80146866 @kind data */
extern u8 D_80146867; /* @source 0x80146867 @kind data */
extern u32 D_8014686C; /* @source 0x8014686C @kind data */
extern u8 D_80146874; /* @source 0x80146874 @kind unknown */
extern u8 D_80146875; /* @source 0x80146875 @kind unknown */

/* Shared main-EXE helpers, both outside this payload and unlifted, so only the
 * call-site prototypes are declared. 0x8015C088 is the front-end startup helper
 * the exact sibling publishModeStatusFromTable5354AndSetFlagBit15World02Area09413_801F2C5C
 * neighbours; 0x8015B5D4 tests one flag of the flag word it is handed and
 * returns nonzero when that flag is set. */
void func_8015C088(void);
s32 func_8015B5D4(u32 arg0, s32 arg1);

/* @source 0x801F2DC4
 * @behavior Publishes the halfword indexed by the signed shared mode byte
 *           D_801490C7 from the target-local halfword table at 0x801F535C into
 *           the shared status halfword D_801490A8 -- the same status store the
 *           exact sibling publishModeStatusFromTable5354AndSetFlagBit15World02Area09413_801F2C5C
 *           makes from the neighbouring table at 0x801F5354 -- and then acts on
 *           that same mode value. Mode 1 runs the front-end startup helper
 *           func_8015C088, stores 0x32 into the shared sub-state byte D_80146875,
 *           clears the scenario progress byte g_ScenarioProgress and the three
 *           shared area state bytes 0x80146865/0x80146866/0x80146867, and
 *           requests primary state 7 through the shared primary-state byte
 *           D_80146874. Mode 2 tests flag 0x20 of the shared flag word
 *           D_8014686C through func_8015B5D4: with the flag clear it runs
 *           func_8015C088, clears the progress and state bytes, stores 0 into
 *           the sub-state byte and requests primary state 9; with the flag set
 *           it runs func_8015C088, stores 2 into the sub-state byte, clears the
 *           progress and state bytes and requests primary state 9. Mode 0, a
 *           negative mode and every other mode publish the status halfword and
 *           leave the progress, state, sub-state and primary-state bytes alone.
 *           Takes no arguments and returns nothing; the 0x18-byte frame keeps
 *           only $ra at 0x10 across the calls.
 *
 *           Constant and store evidence from these original bytes: the mode
 *           cell 0x801490C7 is read once with `lui`/`lb`, the table base
 *           0x801F535C is scaled by `sll` by one and `addu` into $at with
 *           `lhu`, the status cell 0x801490A8 takes the `sh`, and the dispatch
 *           compares the mode with 1, then with `slti 2`, then with 2, so the
 *           three-case switch has an empty case 0 and a dead default. Every
 *           arm keeps its own `jal func_8015C088` and its own store sequence:
 *           the mode-1 arm stores the sub-state byte 0x32 before the four
 *           clears and the primary-state byte 7 after them, the flag-clear arm
 *           stores its sub-state byte 0 before the four clears, and the
 *           flag-set arm stores its sub-state byte 2 before them. The two clear
 *           runs of the mode-1 arm and the flag-set arm share one emitted block
 *           at 0x801F2E98, which is what the tail merge of identical store runs
 *           leaves behind, and the primary-state byte keeps its literal in $v0
 *           across that block. The flag test reads D_8014686C with `lw` into
 *           $a0 and passes 0x20 in $a1.
 *
 *           Name evidence: the name keeps the published table and then spells
 *           the primary-state requests the two live mode arms make, and invents
 *           no scene role of its own. 0x80146874 is the shared primary-state
 *           byte that dispatchPrimaryStateHandlerScenarioScena0200_801F7A60
 *           scales onto the primary-state handler table D_801FE294, 0x80146875
 *           is the shared sub-state byte those handlers read, 0x80146864 is the
 *           shared scenario progress byte and 0x80146865 the shared area status
 *           byte its exact sibling in this target names. The mode-1 arm is the
 *           shape of the exact scenario lift
 *           setProgress1ClearSharedBytes6566AndRequestPrimaryState7ScenarioScena0200_801FD2E0
 *           -- same 0x8015C088 startup call, same progress and state clears,
 *           same primary state 7 -- so the request wording follows proven
 *           in-repo role vocabulary rather than a guess.
 *
 *           Shape: this 0x10C-byte span has no byte-identical sibling. Its
 *           whole body occurs exactly once across the 398 payloads under
 *           out/binaries -- in this payload -- and anchoring on its own three
 *           opening words (frame, saved $ra, `sll $v0,$v1,1`) finds no other
 *           body differing in at most six words, so the clone lever is declined
 *           and the body is seeded from the original bytes. Statement order,
 *           operand forms and declaration style follow the exact sibling
 *           publishModeStatusFromTable5354AndSetFlagBit15World02Area09413_801F2C5C,
 *           which publishes the same status halfword the same way and carries
 *           the same empty case 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusFromTable535cThenRequestPrimaryState7Substate32OrPrimaryState9Substate0Or2ByFlag20World02Area09413_801F2DC4(
    void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F535C[mode];
  switch (mode) {
  case 0:
    break;
  case 1:
    func_8015C088();
    D_80146875 = 0x32;
    g_ScenarioProgress = 0;
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    D_80146874 = 7;
    break;
  case 2:
    if (func_8015B5D4(D_8014686C, 0x20) == 0) {
      func_8015C088();
      g_ScenarioProgress = 0;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      D_80146875 = 0;
      D_80146874 = 9;
    } else {
      func_8015C088();
      D_80146875 = 2;
      g_ScenarioProgress = 0;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      D_80146874 = 9;
    }
    break;
  }
}

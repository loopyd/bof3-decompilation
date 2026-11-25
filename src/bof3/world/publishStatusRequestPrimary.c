#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u16 D_801F5358[]; /* @source 0x801F5358 @kind table */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */
extern u8 D_80146865; /* @source 0x80146865 @kind data */
extern u8 D_80146866; /* @source 0x80146866 @kind data */
extern u8 D_80146867; /* @source 0x80146867 @kind data */
extern u32 D_8014686C; /* @source 0x8014686C @kind data */
extern u8 D_80146874; /* @source 0x80146874 @kind unknown */
extern u8 D_80146875; /* @source 0x80146875 @kind unknown */

/* Shared main-EXE helpers at addresses outside every overlay payload, so only
 * the call-site prototypes are declared and the raw generated names stay.
 * 0x8015C088 is the front-end startup helper the exact siblings in this target
 * call, 0x8015B5D4 tests one flag of the flag word it is handed and returns
 * nonzero when that flag is set, and 0x8015B580 sets one flag of that word. */
void func_8015C088(void);
s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015B580(void* flag_bank, u8 bit_index);

/* @source 0x801F2CB4
 * @behavior Publishes the halfword indexed by the signed shared mode byte
 *           D_801490C7 from the target-local halfword table at 0x801F5358 into
 *           the shared status halfword D_801490A8 -- the same status store the
 *           exact siblings statePublishModeStatus
 *           and
 *           publishModeStatusFromTable535cThenRequestPrimaryState7Substate32OrPrimaryState9Substate0Or2ByFlag20World02Area09413_801F2DC4
 *           make from the neighbouring tables at 0x801F5354 and 0x801F535C --
 *           and then acts only on mode 1. With flag 0x16 of the shared flag word
 *           D_8014686C clear it sets that flag through func_8015B580, runs the
 *           front-end startup helper func_8015C088, clears the scenario progress
 *           byte g_ScenarioProgress, the three shared area state bytes
 *           0x80146865/0x80146866/0x80146867 and the shared sub-state byte
 *           D_80146875, and requests primary state 7 through the shared
 *           primary-state byte D_80146874. With flag 0x16
 *           set it tests flag 0x1A of that same word through func_8015B5D4, runs
 *           func_8015C088 and stores 0x19 into the shared sub-state byte
 *           D_80146875 when the flag is clear or 0x32 when it is set, then
 *           clears the progress and state bytes and requests primary state 7.
 *           Mode 0, a negative mode and every other mode publish the status
 *           halfword and leave the flag word, the progress and state bytes, the
 *           sub-state byte and the primary-state byte alone. Takes no arguments
 *           and returns nothing; the 0x18-byte frame keeps only $ra at 0x10
 *           across the calls.
 *
 *           Constant and store evidence from these original bytes: the mode
 *           cell 0x801490C7 is read once with `lui`/`lb` before the frame is
 *           reserved, the table base 0x801F5358 is scaled by `sll` by one and
 *           `addu` into $at with `lhu`, the status cell 0x801490A8 takes the
 *           `sh`, and the two-case dispatch compares the mode with zero, then
 *           with 1, so the switch has an empty case 0 and a dead default. Both
 *           flag tests re-read the flag word 0x8014686C with `lui`/`lw` into
 *           $a0 rather than keeping it in a register, and pass 0x16 and 0x1A in
 *           $a1. The flag-clear arm keeps the set call `jal func_8015B580`
 *           before `jal func_8015C088`; each inner arm keeps its own
 *           `jal func_8015C088` and materializes its sub-state literal in $v0 in
 *           the delay slot of the jump to the merged store; each arm keeps its
 *           own clear run -- the flag-clear arm clears four bytes and the
 *           sub-state byte while the inner arm stores its sub-state literal
 *           before its four clears. The primary-state literal 7 is materialized
 *           per arm, in the jump delay slot of the flag-clear arm and right
 *           after the inner arm's sub-state store, so only the trailing
 *           `lui`/`sb` pair of `D_80146874 = state` is emitted once and shared
 *           at 0x801F2DAC. That per-arm materialization is what the `state`
 *           local reproduces: writing the literal store once at the join left
 *           the same 272-byte span at 67/68 instructions with the missing
 *           `li $v0,7` first at the join block, and assigning the value in each
 *           arm before the join closed it.
 *
 *           Name evidence: the name keeps the published table, the flag bit the
 *           clear arm sets, the primary-state request both live arms make and
 *           the two sub-state values the inner flag picks between, and invents
 *           no scene role of its own. 0x80146874 is the shared primary-state
 *           byte that dispatchPrimaryStateHandlerScenarioScena0200_801F7A60
 *           scales onto the primary-state handler table D_801FE294, 0x80146875
 *           is the shared sub-state byte those handlers read, 0x80146864 is the
 *           shared scenario progress byte, and 0x80146865 the shared area state
 *           byte its exact sibling in this target names. Flags 0x16 and 0x1A are
 *           the shared progression flags the same vocabulary reports elsewhere
 *           in the tree (for example
 *           func_8015B5D4(D_8014686C, 0x16) and func_8015B5D4(D_8014686C, 0x1A)
 *           in requestFrontEndStateForWorldFrontSideAndPositionZoneScenarioScena1400_801FB84C),
 *           so the wording follows proven in-repo role vocabulary rather than a
 *           guess.
 *
 *           Shape: this 0x110-byte span has no byte-identical sibling anywhere.
 *           Its full span occurs once across the 398 payloads under out/binaries,
 *           and its own 0x20-byte opening run (frame-independent `lui`/`lb` of
 *           0x801490C7, the `sll`, the table `addu`, the status `sh` and the
 *           two-case dispatch) also occurs once, so the clone lever is declined
 *           and the body is seeded from the original bytes. Statement order,
 *           operand forms and declaration style follow the exact sibling
 *           publishModeStatusFromTable535cThenRequestPrimaryState7Substate32OrPrimaryState9Substate0Or2ByFlag20World02Area09413_801F2DC4,
 *           which publishes the same status halfword the same way, carries the
 *           same empty case 0 and the same 0x8015C088 / 0x8015B5D4 / clear-run
 *           statement shapes. No `jal` immediate targets this address and the
 *           single word naming it is the handler-table entry at 0x801F52F0, so
 *           it is a script-invoked handler and its roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishStatusRequestPrimary(
    void) {
  s32 mode;
  s32 state;

  mode = D_801490C7;
  D_801490A8 = D_801F5358[mode];
  switch (mode) {
  case 0:
    break;
  case 1:
    if (func_8015B5D4(D_8014686C, 0x16) == 0) {
      func_8015B580(D_8014686C, 0x16);
      func_8015C088();
      g_ScenarioProgress = 0;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      D_80146875 = 0;
      state = 7;
    } else {
      if (func_8015B5D4(D_8014686C, 0x1A) == 0) {
        func_8015C088();
        D_80146875 = 0x19;
      } else {
        func_8015C088();
        D_80146875 = 0x32;
      }
      state = 7;
      g_ScenarioProgress = 0;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
    }
    D_80146874 = state;
    break;
  }
}

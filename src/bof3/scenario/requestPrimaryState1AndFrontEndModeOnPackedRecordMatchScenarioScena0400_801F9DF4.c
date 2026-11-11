#include "bof3/bof3.h"

/* Shared scenario bank word of the main executable that this overlay's stage
 * waits probe and commit; the original loads it with one `lw` before each of
 * the three probes.
 * @source 0x8014686C @kind unknown */
extern u32 D_8014686C;
/* Shared primary state byte of the main executable: this overlay's primary
 * state dispatcher dispatchPrimaryStateHandlerScenarioScena0400_801F863C
 * indexes its handler table at 0x801FA210 with it, so the 1 stored here selects
 * primary state 1. Stored with one `sb`.
 * @source 0x80146874 @kind unknown */
extern u8 D_80146874;
/* Sub-state byte of the same shared state tuple, stored with one `sb`.
 * @source 0x80146875 @kind unknown */
extern u8 D_80146875;
/* Halfword of the same shared state tuple, stored with one `sh`.
 * @source 0x80146876 @kind unknown */
extern u16 D_80146876;
/* Leading byte of the main-RAM game-mode record table 0x80145E98, taken here
 * with one `lbu`; etc/game/00 (func_801C090C) publishes it.
 * @source 0x80145E98 @kind unknown */
extern u8 D_80145E98[];
/* Scratchpad byte at the base of the 0x1F800000 scratch area, where the
 * front-end helpers mirror the game-mode byte; stored with one `sb`.
 * @source 0x1F800000 @kind data */
extern u8 D_1F800000[];

/* Shared stage wait, stage commit and front-end startup helpers of the main
 * executable, outside this payload. */
s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C088(void);
void func_8015DF18(u32 cue);

s32 queueFrontEndModeAndRequestPrimaryMode2ScenarioScena0400_801F9FD0(s32 arg0);

/* @source 0x801F9DF4
 * @behavior Packed-record dispatcher of this overlay. Its only caller is
 *   forwardRoute28RecordScenarioScena0400_801F9DB4 (0x801F9DB4), which reaches
 *   it through the jal at 0x801F9DD4 once the shared route word D_80143F00
 *   reads 0x28 and forwards both incoming packed record words unchanged; no
 *   image word holds this address, so that jal is its only reference. The body
 *   saves $a0 in $s0 and $a1 in $s1 before any call and never writes either.
 *   It probes the shared scenario bank D_8014686C with the stage wait
 *   func_8015B5D4 and recognises one packed record pair per armed stage:
 *   stage 2 (wait returns zero) takes a first word type in 0x57..0x5A with a
 *   second word type in 0x13..0x15 while the game-mode byte D_80145E98 reads
 *   neither 5 nor 6 nor 4 - it mirrors that byte into scratchpad 0x1F800000
 *   before testing it, since the store sits between the two tests - and then
 *   runs the front-end startup helper func_8015C088, commits bank stage 2 with
 *   func_8015B580, submits cue 0x204 through func_8015DF18, stores 0x1E into the
 *   shared halfword D_80146876, requests primary state 1 with sub-state 1
 *   (D_80146874 = 1, D_80146875 = 1) and returns 1.
 *   Stage 3 (the stage-2 wait reports nonzero and the stage-3 wait zero) takes a
 *   first word type in 0x57..0x5A with a second word type in 0xB..0x10 under the
 *   same three-mode test, requests primary state 1 with sub-state 0 and queues
 *   front-end mode 2; every other arrival from that stage - a rejected type, or
 *   a game-mode byte reading 4, 5 or 6 - falls into a shared test that a first
 *   word of exactly 0x4D8000 with a second word type of 0x18 or 0x19 takes to
 *   the same state stores with queued mode 3.
 *   Stage 7 (both earlier waits report nonzero and the stage-7 wait zero) takes
 *   a first word type in 0x1F..0x21 with a second word of exactly 0x248000 to
 *   the same state stores with queued mode 0x26.
 *   Every queued path returns the front-end tail's result sign-extended from its
 *   low byte (sll/sra 24, `andi 0xFF` on the queued argument inside the tail),
 *   and every other path returns 0. The 0x20-byte frame keeps $s0, $s1 and $ra
 *   across the calls, the D_8014686C word load is scheduled between the second
 *   saved-argument copy and the $ra store, and the three queued arms' identical
 *   D_80146874/D_80146875 store pair and tail call merge into one block whose
 *   mode argument is materialised in the delay slot of the arm that takes it (2
 *   at 0x801F9F28, 3 at 0x801F9F48, 0x26 at 0x801F9F88).
 *
 * Name evidence: D_80146874 is the shared primary state byte that this overlay's
 *   dispatchPrimaryStateHandlerScenarioScena0400_801F863C indexes its handler
 *   table with, and D_80146875 is the sub-state byte of the same tuple in this
 *   target's queueFrontEndModeAndRequestPrimaryMode2ScenarioScena0400_801F9FD0
 *   (0x801F9FD0), so the three queued arms request primary state 1 with a
 *   cleared sub-state and the stage-2 arm requests primary state 1 with
 *   sub-state 1. The queued mode reaches the front-end mode request of the main
 *   executable through that sibling, whose own prose records the 3 and 0x26
 *   arguments of this body. The two packed words carry no evidenced role beyond
 *   their high-half type ranges and their two whole-word matches (0x4D8000 and
 *   0x248000), so they stay structural.
 *
 * Shape: seeded from this target's original bytes. The SCENA03 00 overlay's
 *   exact lifts requestPrimaryState1Or2OnPackedRecordMatchScenarioScena0300_801FC7EC
 *   (0x801FC7EC) and requestPrimaryState6Substate5Or0DOr0FAndQueueFrontEndMode0COr11ScenarioScena0300_801FCB4C
 *   (0x801FCB4C) supply the `(u32)((word >> 16) - base) >= width` reject form,
 *   the `func_8015B5D4(D_8014686C, stage) == 0` branch chain, the
 *   `(s8)queueFrontEndMode...` tail and its per-arm mode constant, and
 *   stageRoutePath2Setup (0x801F6F30) the scratchpad mirror of a front-end byte
 *   and the func_8015C088/func_8015B580 startup pair. No byte-identical twin
 *   exists: the 0x1DC-byte span's discriminating immediates 0x4D8000, 0x248000,
 *   0x204 and 0x1E plus its three queued mode constants occur only in this
 *   payload, and the overlay's nearest analogue
 *   func_801FCD68 in the scena05 00 overlay is 0x3A8 bytes and gates on its own
 *   route word, so the clone lever is ineligible and this body is shape
 *   evidence only.
 * @status partial
 * @match 91.60
 * @residual The state byte's zero extension. This body reuses the compiler's
 *   QImode-to-SImode promotion of the game-mode byte for the range reject, so
 *   each state-gated arm opens with `andi $v1,$v0,0xFF` (raw byte in $v0) ahead
 *   of the `sb`, where the original computes the reject on the raw `lbu`
 *   register in $v1 and issues its single `andi $v1,$v1,0xFF` only on the
 *   fallthrough, before `li $v0,4` and `beq`. That 5-instruction block in each
 *   of the two arms is the whole residual: 109/119 instructions at the
 *   original's 476 bytes, first difference the `lui` of that load. Measured
 *   clean-C shapes that all keep this shape or lose the `andi` entirely (468
 *   bytes): u8 local with an explicit mask, a plain `state == 4`, a
 *   `(u32)state` cast at the use, `(u8)` casts from u32/s32 locals, separate
 *   QImode and SImode copies of one load, a named delta local, the two tests
 *   combined with `||`, and a volatile byte view. Every SImode spelling folds
 *   the widening (the load's own zero extension is known), so only the QImode
 *   local carries the `andi`, and the range reject then consumes it.
 */
s32 requestPrimaryState1AndFrontEndModeOnPackedRecordMatchScenarioScena0400_801F9DF4(
    s32 arg0, s32 arg1) {
  u8 state;

  if (func_8015B5D4(D_8014686C, 2) == 0) {
    if ((u32)((arg0 >> 16) - 0x57) < 4u &&
        (u32)((arg1 >> 16) - 0x13) < 3u) {
      state = D_80145E98[0];
      D_1F800000[0] = state;
      if ((u32)(state - 5) < 2u) {
        return 0;
      }
      if (state == 4) {
        return 0;
      }
      func_8015C088();
      func_8015B580(D_8014686C, 2);
      func_8015DF18(0x204);
      D_80146876 = 0x1E;
      D_80146874 = 1;
      D_80146875 = 1;
      return 1;
    }
    return 0;
  }

  if (func_8015B5D4(D_8014686C, 3) == 0) {
    if ((u32)((arg0 >> 16) - 0x57) < 4u &&
        (u32)((arg1 >> 16) - 0xB) < 6u) {
      state = D_80145E98[0];
      D_1F800000[0] = state;
      if ((u32)(state - 5) >= 2u && state != 4) {
        D_80146874 = 1;
        D_80146875 = 0;
        return (s8)queueFrontEndModeAndRequestPrimaryMode2ScenarioScena0400_801F9FD0(2);
      }
    }
    if ((u32)((arg1 >> 16) - 0x18) < 2u && arg0 == 0x4D8000) {
      D_80146874 = 1;
      D_80146875 = 0;
      return (s8)queueFrontEndModeAndRequestPrimaryMode2ScenarioScena0400_801F9FD0(3);
    }
  } else if (func_8015B5D4(D_8014686C, 7) == 0) {
    if ((u32)((arg0 >> 16) - 0x1F) < 3u && arg1 == 0x248000) {
      D_80146874 = 1;
      D_80146875 = 0;
      return (s8)queueFrontEndModeAndRequestPrimaryMode2ScenarioScena0400_801F9FD0(0x26);
    }
  }
  return 0;
}

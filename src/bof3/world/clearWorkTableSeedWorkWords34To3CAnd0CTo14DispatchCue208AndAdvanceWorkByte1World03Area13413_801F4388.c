#include "bof3/bof3.h"

void clearWorkTableAt800E4800World03Area13413_801F4974(void);

/* Shared published work-pointer cell: the original re-materialises the cell
 * address and reloads the pointer for the saved-word group and again for the
 * state-byte advance, so the fixed-address cell symbol keeps both loads. */
extern u8* volatile D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; it consumes one cue id and returns
 * nothing.
 */
extern void dispatchSoundCue(u32 cue_id);

/* @source 0x801F4388
 * @behavior Overlay run initialiser: entry 0 of the work-byte-1 handler table
 * based at 0x801F5828 that this target's exact dispatcher
 * dispatchStateByte1Table5828World03Area13413_801F4344 indexes with the work
 * record's state byte at +1. That base is the fourth word of the pointer run
 * starting at 0x801F581C, so this address is also word 3 of the eleven-word run
 * the sibling dispatcher dispatchStateByte1Table581cWorld03Area13413_801F41D0
 * documents; the shipped run reads 0x801F4214, 0x801F4288, 0x801F4310,
 * 0x801F4388, 0x801F4400 and on to 0x801F46CC before the non-pointer word 1 at
 * 0x801F5848.
 * The body first calls the work-table clear helper at 0x801F4974, the routine
 * that zeroes the lead byte of the 0x20 entries of the 0x28-byte-stride record
 * table at 0x800E4800. It then reloads the work record published at the
 * scratchpad pointer cell 0x1F800044 and seeds it for the interpolation its
 * following entries run: it copies the record's saved words at +0x34, +0x38 and
 * +0x3C into its words at +0x0C, +0x10 and +0x14 and starts the interpolated
 * word at +0x1C from that same +0x3C copy, clears the halfword at +0x20, stores
 * 0x01000000 in the word at +0x24 - the step the next handler
 * advanceWorkWord1CClampedToWord14AndRearmCountdownThenAdvanceWorkByte1World03Area13413_801F4400
 * adds to +0x1C and clamps against +0x14 - and arms the countdown byte at +0x09
 * with 0x40, four times that handler's 0x10 re-arm. It finally reads the cell
 * again, advances the record's state byte at +0x01 by one so the next frame
 * dispatches entry 1 at 0x801F4400, and dispatches sound cue 0x208 through the
 * resident executable cue dispatcher at 0x8015DF18, whose call delay slot keeps
 * the state-byte store. Takes no arguments and returns nothing; its 0x18-byte
 * frame only holds $ra at 0x10 across the two calls, whose delay slots stay a
 * nop and that store respectively. The roles of the record's words at +0x14 and
 * +0x1C, of its halfword at +0x20 and of the 0x01000000 step stay described by
 * offset.
 *
 * Name evidence: the only in-image references to 0x801F4388 are the pointer-run
 * words described above; there is no jal caller, which is the dispatch shape
 * rather than missing evidence. The clear-then-seed-then-advance role comes from
 * this body's own literals, from the two helpers it calls and from the field
 * seeding that the already-documented entry 1 at 0x801F4400 continues.
 *
 * Shape evidence: the cell-reload points, the state-byte advance before the cue
 * call and the block-local scalar seeding follow this target's exact
 * claimFreeWorkRecordSeedWorkPositionWorld03Area13413_801F48B0 and
 * clearWorkTableOnFirstStepArmCountdownStampStepCountAndAdvanceWorkByte1World03Area13413_801F4214,
 * and the scenario overlay's exact
 * tickCountdownByte9SeedWorkWords34To3CAnd0CTo14AdvanceStateAndQueueCues202And203ScenarioScena0800_801F9AC0
 * is the same seed-words-then-advance-state-then-cue shape with its own
 * literals; this function's offsets, constants and call targets come from its
 * original bytes.
 *
 * Matching note: the saved-word group has to be read before the countdown byte
 * is armed, and the original keeps the three loads immediately after the cell
 * load, so its load delay stays an unfilled nop while the arm store follows.
 * Writing the three loads as plain statements ahead of the store measured four
 * bytes short: the loads are single-definition pseudos whose value is live, so
 * sched.c's birthing_insn_p/adjust_priority raise them to max_priority and they
 * sink past the arm store and the cell reload, which fills the load delay slot
 * instead (26/30, 120->116, first mismatch at +0x18). A single-execution scope
 * around the real load group puts a loop note between that group and the arm
 * store, whose pending-use and pending-read dependencies pin the loads there;
 * that measured 30/30 at 120->120 bytes with no remaining difference.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableSeedWorkWords34To3CAnd0CTo14DispatchCue208AndAdvanceWorkByte1World03Area13413_801F4388(
    void) {
  u8* work;
  u8* state;
  s32 at34;
  s32 at38;
  s32 at3c;

  clearWorkTableAt800E4800World03Area13413_801F4974();
  do {
    work = D_1F800044;
    at34 = *(s32*)(work + 0x34);
    at38 = *(s32*)(work + 0x38);
    at3c = *(s32*)(work + 0x3C);
  } while (0);
  work[9] = 0x40;
  state = D_1F800044;
  *(u16*)(work + 0x20) = 0;
  *(s32*)(work + 0x24) = 0x1000000;
  *(s32*)(work + 0x0C) = at34;
  *(s32*)(work + 0x10) = at38;
  *(s32*)(work + 0x14) = at3c;
  *(s32*)(work + 0x1C) = at3c;
  state[1]++;
  dispatchSoundCue(0x208);
}

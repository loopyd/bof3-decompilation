#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; it consumes one cue id and returns
 * nothing.
 */
extern void dispatchSoundCue(u32 cue_id);

/* @source 0x801D2BA8
 * @behavior Entry 6 of this overlay's step-handler code-pointer run at
 * 0x801D3954, which the overlay entry func_801D0C04 selects with the unsigned
 * work byte at offset +0x1 of the scratchpad work object published at pointer
 * slot 0x1F800044 and calls with no arguments and no result consumed. It arms
 * that object's countdown byte at +0x9 with 0x14, clears the word pair at
 * +0x0C/+0x10 - the first two words of the step triple the sibling
 * advanceWorkByte6ClaimRecordInvokeTwoHelpersAndOnProgress20RestoreMotionStateAndAdvanceWorkByte1ScenarioSce15ef100_801D0F10
 * clears whole - and the word pair at +0x18/+0x1C, the first two words of the
 * head point that same sibling advances by that step triple. It then advances
 * the work byte at +0x1 by one, so the next dispatch selects entry 7
 * (0x801D2C00), and dispatches sound cue 0x208 through the resident executable
 * cue dispatcher at 0x8015DF18. The scratchpad cell is read twice because the
 * byte store through the loaded pointer may alias that cell: the countdown arm
 * and the four word clears run through the first load in $v1, and the
 * state-byte advance runs through the second load in $a0. Takes no arguments
 * and returns nothing; its 0x18-byte frame exists only to keep $ra at 0x10
 * across the cue call, whose delay slot holds the 0x208 argument load.
 *
 * Name evidence: the only in-image reference to 0x801D2BA8 is word 6 of the
 * ten-word run at 0x801D3954, which the exact dispatcher
 * dispatchWorkByte1HandlerScenarioSce15ef100_801D2170 indexes with work byte
 * +0x1; there is no jal caller, which is the dispatch shape rather than missing
 * evidence. The arm-clear-advance-cue roles come from this body's own literals
 * and offsets and from the field roles the already-reviewed entries of the same
 * run document.
 *
 * Shape evidence: the pointer-cell spelling, the second cell load into a
 * separate `next` local, the clears written through the first load and the
 * trailing state-byte advance are the exact lift
 * armCountdown10ClearScreenPairAndAdvanceWorkByteScenarioSce15ef200_801D0F68
 * (0x801D0F68 in emi/scenario/sce15ef2/00), whose body is this statement
 * sequence with two halfword clears; the cue dispatch after the state advance
 * is the one the exact sibling
 * clearWorkTableSeedWorkWords34To3CAnd0CTo14DispatchCue208AndAdvanceWorkByte1World03Area13413_801F4388
 * (0x801F4388 in emi/world03/area134/13) makes with the same cue 0x208. This
 * function's own offsets, countdown value and cue id come from its original
 * bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearStepAndHeadXYWordsArmCountdown20DispatchCue208AndAdvanceWorkByte1ScenarioSce15ef100_801D2BA8(void) {
  u32 slot_offset;
  u8* work;
  u8* next;

  slot_offset = 0x44u;
  work = PSX_REF(u8*, SPAD_BASE + slot_offset);
  work[9] = 0x14;
  next = PSX_REF(u8*, SPAD_BASE + slot_offset);
  *(u32*)(work + 0x0C) = 0;
  *(u32*)(work + 0x10) = 0;
  *(u32*)(work + 0x18) = 0;
  *(u32*)(work + 0x1C) = 0;
  next[1]++;
  dispatchSoundCue(0x208);
}

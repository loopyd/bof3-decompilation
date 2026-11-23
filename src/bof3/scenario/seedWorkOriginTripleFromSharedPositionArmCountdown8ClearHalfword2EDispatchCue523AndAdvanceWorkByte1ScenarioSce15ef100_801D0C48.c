#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; it consumes one cue id and returns
 * nothing.
 */
extern void dispatchSoundCue(u32 cue_id);

/* Shared main-executable position triple: other overlays hand the same three
 * words to their projection helpers, and the payload entry of this overlay
 * publishes them as the player position. */
/* @source 0x80145EC4 @kind unknown */
extern u32 D_80145EC4;
/* @source 0x80145EC8 @kind unknown */
extern u32 D_80145EC8;
/* @source 0x80145ECC @kind unknown */
extern u32 D_80145ECC;

/* @source 0x801D0C48
 * @behavior Entry 0 of the overlay step-handler code-pointer run at
 * 0x801D3918, whose word 0 holds this address and whose entry 1 (0x801D0CC0)
 * starts from the state this body leaves. It copies the shared position triple
 * at 0x80145EC4/0x80145EC8/0x80145ECC into the scratchpad work object origin
 * words at +0x64/+0x68/+0x6C, with the third word lifted by 0x800000, then
 * arms the countdown byte at +0x9 with 8 and clears the halfword at +0x2E. It
 * re-reads the pointer cell 0x1F800044 after those stores because the stores
 * through the loaded pointer may alias it, advances the work byte at +0x1
 * through that reloaded pointer and dispatches sound cue 0x20B. The work-byte
 * advance is what moves the dispatcher func_801D0C04 from this entry to entry
 * 1 on the next frame. It takes no arguments, returns nothing and creates its
 * 0x18-byte frame only to keep $ra at 0x10 across the cue call.
 *
 * Evidence: word 0 of the run at 0x801D3918 (payload offset 0x2D18) is
 * 0x801D0C48, so this span is element 0 of the table that func_801D0C04 loads
 * after scaling the work byte at +0x1 by four. The countdown byte the tail
 * re-arms, the origin triple the stores fill and the +0x1 advance are the ones
 * entry 1 reads at 0x801D0CC0, so the two bodies are consecutive steps of one
 * walk.
 *
 * Shape evidence: the exact lift
 * seedWorkWords64AndAdvanceStateWorld02Area09813_801F3814 (0x801F3814 in
 * emi/world02/area098/13) writes the same three shared words to the same work
 * offsets, arms byte +0x9 with 8, clears halfword +0x2E through a re-read of
 * the pointer cell and advances byte +0x1 with the same registers and operand
 * forms; its words match this span up to the frame this body needs for the cue
 * call, which replaced its load-delay nop, and up to the trailing call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkOriginTripleFromSharedPositionArmCountdown8ClearHalfword2EDispatchCue523AndAdvanceWorkByte1ScenarioSce15ef100_801D0C48(void) {
  u8* work;
  u8* next;
  u32 origin_z;

  work = D_1F800044;
  *(u32*)(work + 0x64) = D_80145EC4;
  *(u32*)(work + 0x68) = D_80145EC8;
  origin_z = D_80145ECC;
  work[9] = 8;
  next = D_1F800044;
  *(u16*)(work + 0x2E) = 0;
  *(u32*)(work + 0x6C) = origin_z + 0x800000u;
  next[1]++;
  dispatchSoundCue(0x20B);
}

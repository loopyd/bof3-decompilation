#include "bof3/bof3.h"

void func_801F30B0(u8* record);
void func_801F31A0(u8* record);
void func_801F33DC(u8* record);
void func_801F3870(void);

/* Shared saved work triplet, read as three consecutive 32-bit words by the
 * overlay state handlers of this target. Declared plain: the original loads
 * each word immediately before the store that consumes it. */
extern s32 D_80146954;
extern s32 D_80146958;
extern s32 D_8014695C;

/* Work record pointer published at the scratchpad cell 0x1F800044. Declared
 * plain, not volatile: the original re-materialises the cell address and
 * reloads the pointer for each of the three accesses in the countdown group,
 * which the plain cell already forces, and it keeps the two-instruction
 * `lui`/`lw` cell form that a cast to the literal address does not. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F44A0 @kind data
 * Base pointer of the work record this overlay publishes for itself: entry 0
 * of the handler run at 0x801F3DAC stores the literal 0x800E4800 into this
 * cell, and entries 4 and 5 read it back as the base to seed and to render.
 * The cell sits in the payload tail, so its image value is zero until entry 0
 * runs. */
extern u8* D_801F44A0;

/* @source 0x801F2F50
 * @behavior Overlay step handler, entry 4 of the six-word handler run based at
 *           0x801F3DAC, which this target dispatcher
 *           dispatchStateByte1Table3dacWorld02Area08013_801F2C04 indexes with
 *           the state byte at +1 of the work record published at the
 *           scratchpad pointer cell 0x1F800044. Loads the record base the
 *           payload publishes in D_801F44A0 and copies the three shared saved
 *           words D_80146954, D_80146958 and D_8014695C into the words at
 *           +0x10, +0x14 and +0x18 of that record, then calls the three
 *           overlay record helpers at 0x801F30B0, 0x801F31A0 and 0x801F33DC,
 *           each with the reloaded record base as its only argument, and
 *           finally the eight-record table step loop at 0x801F3870 with no
 *           arguments. It then decrements the countdown byte at +0x09 of the
 *           scratch work record; when that countdown reads zero it re-arms the
 *           byte with 0x10 and increments the state byte at +0x01 so the next
 *           frame dispatches entry 5 at 0x801F3018. Takes no arguments and
 *           returns nothing; its 0x18-byte frame holds $ra at 0x10 across the
 *           calls.
 *
 * Name evidence: the only reference to 0x801F2F50 in the payload is word 4 of
 * the handler run at 0x801F3DAC, so there is no jal caller and the dispatch
 * shape is the naming route, not missing evidence. That run is indexed with the
 * work record state byte at +1 by this target exact dispatcher
 * dispatchStateByte1Table3dacWorld02Area08013_801F2C04. The seed role comes
 * from the three stored words and from entry 0 of the same run, which loads the
 * same triplet from the shared state and both caches it in the scratch work
 * record and publishes the D_801F44A0 base the later entries seed; the countdown
 * re-arm and state-byte increment come from the byte cells this body writes.
 * The roles of the three record helpers stay described by address: none of them
 * is lifted, and their own bodies are needed before any semantic claim.
 *
 * Shape evidence: statement order, operand forms and the reload point of the
 * record base follow the exact sibling
 * seedWorkWords34To3CAnd0CTo14DispatchCue207AndAdvanceWorkByte1World03Area13213_801F30C0,
 * which copies a saved word triplet into a record and then advances the state
 * byte, and the countdown group follows the exact
 * decrementCountdownAdd4WorkWords64And68World02Area08213_801F2C9C and
 * advanceWorkWord1CClampedToWord14AndRearmCountdownThenAdvanceWorkByte1World02Area10313_801F37A0.
 * This function's own offsets and literals come from its original bytes.
 * The clone lever is declined on one payload proof: the leading 0x40 bytes of
 * these 0xC8 bytes occur only at payload offset 0x350 of the 396 payloads under
 * out/binaries, so no byte-identical twin of the range exists anywhere in the
 * tree.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedPublishedRecordWordsFromSavedTripletAndRearmCountdownThenIncrementWorkByte1World02Area08013_801F2F50(
    void) {
  u8* record;
  u8* cursor;
  u8 countdown;

  record = D_801F44A0;
  *(s32*)(record + 0x10) = D_80146954;
  *(s32*)(record + 0x14) = D_80146958;
  *(s32*)(record + 0x18) = D_8014695C;
  func_801F30B0(record);
  func_801F31A0(D_801F44A0);
  func_801F33DC(D_801F44A0);
  func_801F3870();
  cursor = D_1F800044;
  countdown = (u8)(cursor[9] - 1);
  cursor[9] = countdown;
  if (countdown == 0) {
    D_1F800044[9] = 0x10;
    D_1F800044[1]++;
  }
}

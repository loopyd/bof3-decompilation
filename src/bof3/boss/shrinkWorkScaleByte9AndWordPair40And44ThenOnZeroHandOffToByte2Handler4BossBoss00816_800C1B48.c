#include "bof3/bof3.h"

/* Work-object pointer cell in scratchpad: the battle code publishes the scratch
 * work record here and every access in this overlay reaches it through this
 * cell. The original re-reads the cell for the first draw call, for the pair of
 * clamped words and for each of the five re-arm stores, because a store through
 * the loaded pointer may alias the cell, so every access names the symbol
 * instead of keeping one pointer in a local. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Overlay renderer at 0x800C236C: it walks 64 angles, multiplies the sine and
 * cosine of each by this argument, shifts the products down by 12 and submits
 * the resulting ring of vertices to the primitive cursor, so the argument is
 * the ring's scale. It takes the byte this handler loads from work byte +9 and
 * returns nothing. */
void func_800C236C(u8 scale); /* @source 0x800C236C */

/* Shared main-executable lookup helper: its exact lift at 0x8014DD3C applies the
 * main lookup record whose id matches its argument to the active work record, so
 * the argument here is a record id. This target's generated
 * undefined_funcs_auto row binds the address-anchored name. */
void func_8014DD3C(s32 entry); /* @source 0x8014DD3C */

/* Resource request helper of the main executable, reached through the same
 * generated undefined_funcs_auto row; the sibling boss lifts declare the same
 * prototype and pass a resource id. */
void func_8014D6B8(u32 resource); /* @source 0x8014D6B8 */

/* Shared main-program enemy accumulation pass: it raises scratch work flag
 * 0x40, folds the current enemy work halfwords into the global accumulators,
 * submits the scratch work and re-arms scratch work bytes +1..+3 as 2, 0 and 0.
 * This handler clears the flag it raises and replaces that re-arm. */
void accumulateEnemyHalfwordsAndResetScratch(void); /* @source 0x801E54EC */

/* @source 0x800C1B48
 * @behavior Per-frame handler of this overlay's work state machine, entry 11 of
 *           the fifteen-word run based at 0x800C2690 that
 *           dispatchStateByte2Table2690BossBoss00816_800C18A0 indexes with work
 *           byte +2. The word 0x800C26BC is its only in-image reference and no
 *           `jal` reaches it, so that dispatcher is its only caller; the 0x18-byte
 *           frame keeps $ra at 0x10 across the four calls and the plain `jr $ra`
 *           return path places its end.
 *           While work byte +9 is set it runs the overlay renderer at 0x800C236C
 *           with that byte, which draws a 64-point ring scaled by it, then
 *           subtracts 4 from the byte and subtracts 0x2000 from each of the words
 *           at +0x40 and +0x44, clamping a word that is not positive to 0 instead,
 *           so the ring and that pair shrink together each frame.
 *           Once work byte +9 reaches 0 it ends the effect: it clears work byte
 *           +0x48, applies main lookup record 0x83 through 0x8014DD3C, requests
 *           resource 0 through 0x8014D6B8, runs the shared accumulation pass at
 *           0x801E54EC, and then overrides the re-arm that pass writes -- it
 *           clears bit 0x40 the pass raises in work byte 0, stores 6 into work
 *           byte +1, the index that the dispatcher run based at 0x800C2660
 *           answers with this overlay's byte-2 dispatcher 0x800C18A0, stores 4
 *           into work byte +2, the index of func_800C18E4 in that byte-2 run,
 *           advances work byte +3, which the pass has just reset to 0, to 1, and
 *           clears work byte +4.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void shrinkWorkScaleByte9AndWordPair40And44ThenOnZeroHandOffToByte2Handler4BossBoss00816_800C1B48(
    void) {
  func_800C236C(D_1F800044[9]);
  if (D_1F800044[9] == 0) {
    D_1F800044[0x48] = 0;
    func_8014DD3C(0x83);
    func_8014D6B8(0);
    accumulateEnemyHalfwordsAndResetScratch();
    D_1F800044[0] &= 0xBF;
    D_1F800044[1] = 6;
    D_1F800044[2] = 4;
    D_1F800044[3]++;
    D_1F800044[4] = 0;
  } else {
    D_1F800044[9] -= 4;
    if (FIELD_REF(s32, D_1F800044, 0x40u) > 0) {
      FIELD_REF(s32, D_1F800044, 0x40u) -= 0x2000;
    } else {
      FIELD_REF(s32, D_1F800044, 0x40u) = 0;
    }
    if (FIELD_REF(s32, D_1F800044, 0x44u) > 0) {
      FIELD_REF(s32, D_1F800044, 0x44u) -= 0x2000;
    } else {
      FIELD_REF(s32, D_1F800044, 0x44u) = 0;
    }
  }
}

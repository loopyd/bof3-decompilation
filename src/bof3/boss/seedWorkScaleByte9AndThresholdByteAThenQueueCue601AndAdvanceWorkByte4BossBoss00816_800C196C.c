#include "bof3/bof3.h"

/* Work-object pointer cell in scratchpad: the battle code publishes this
 * overlay work record here and every access in this overlay reaches it through
 * this cell. The original re-reads the cell for the second byte store and again
 * after the call, because a store through the loaded pointer may alias the
 * cell, so every access names the symbol instead of keeping one pointer in a
 * local. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared main-program sound-cue dispatcher, carried by the main executable and
 * declared with the cue-id width its reviewed call sites use; this target's
 * generated undefined_funcs_auto row binds the address-anchored name. */
void func_8015DF18(u16 cue_id); /* @source 0x8015DF18 */

/* @source 0x800C196C
 * @behavior Entry 0 of the nine-word handler run based at 0x800C26B0, the run
 * that dispatchStateByte4Table26b0BossBoss00816_800C1928 indexes with the work
 * record dispatch byte at +4, and the only in-image word that names this
 * function. It seeds the two work bytes the appear sequence below it animates:
 * byte +9, which the ring renderer at 0x800C236C takes as its scale, and byte
 * +0x0A, the threshold the next entry compares that byte against and halves
 * each time the two meet, both take 0x40. It then plays sound cue 0x601 through
 * the shared cue dispatcher at 0x8015DF18 and increments work byte +4, which
 * hands the record to entry 1 of the same run at 0x800C19C0. Takes no arguments
 * and returns nothing; the 0x18-byte frame keeps $ra at 0x10 across the one
 * call, and the byte store that follows the call is its delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkScaleByte9AndThresholdByteAThenQueueCue601AndAdvanceWorkByte4BossBoss00816_800C196C(
    void) {
  D_1F800044[9] = 0x40;
  D_1F800044[0x0A] = 0x40;
  func_8015DF18(0x601);
  D_1F800044[4]++;
}

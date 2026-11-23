#include "bof3/bof3.h"

/* Work-record pointer cell of this overlay. func_801172D0 seeds a record in it
 * and the record pass func_80117900 walks the sixteen records of the 0x800E4800
 * table by repointing it. The word is zero in the image and lies inside the
 * Splat textbin row D_8011818C. A store through the record pointer can alias the
 * cell, so the original re-reads it after the seeding call instead of reusing
 * the pointer it published. */
extern u8* workRecordCursor; /* @source 0x801181DC @kind data */

/* Shared per-frame phase word of the main executable: several lifted bodies in
 * this target take its low bits as the phase of a frame cycle, here the low two
 * bits that make this handler's record claim run on every fourth frame. Its
 * wider role is not localised by this payload, so it stays address-anchored.
 * @source 0x80143E6C @kind unknown
 */
extern u32 D_80143E6C;

/* Scratchpad work-object pointer cell. The original materialises the cell with
 * `lui` plus a 0x44 offset load, and it reloads the cell for the countdown and
 * for the state advance, because a store through the object pointer can alias
 * it. The work object's word at 0x0C carries the height this handler lowers.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Both callees are lifted members of this overlay's own code spans. */
u8* queryFreeWorkSlot(void);
void claimRecordSeedPosition(u8* record);

/* @source 0x80117768
 * @behavior Step handler 6 of the thirteen in-image pointers the Splat textbin
 * row D_8011818C places: word 6 at 0x801181A4, the base
 * callRunRecordPass indexes
 * from the work object's byte at offset 1 once that byte reads 6. On the frames
 * whose shared phase word 0x80143E6C holds zero in its low two bits it claims
 * the first record that is free in this overlay's 0x800E4800 work table, the
 * sixteen records of 0x28-byte stride whose lead byte is the claim flag,
 * publishes the record in the cursor workRecordCursor, seeds it
 * through claimRecordSeedPosition - which
 * clears the record's word at 0x1C - and then takes the work object's word at
 * 0x0C, the 0x200000 base the previous handler writes there, lowers it by
 * 0x10000 and stores the result both back into that object word and into the
 * record word at 0x1C. Every frame it then counts down the scratch work object's
 * byte at offset 9, the countdown the state-4 handler
 * raiseFlagReseedRecords
 * arms to 0xB4 and the state-5 handler
 * claimEveryFourthFrame
 * re-arms to 0x40, and when that byte reaches zero it increments the object's
 * byte at 1 so the state-7 handler at 0x80117818 runs from the next frame on.
 * The phase word load precedes frame creation, the 0x18-byte frame only keeps
 * $ra at 0x10 across the two calls, the work object's word at 0x0C is read once
 * and the cell is reloaded for the countdown and for the state advance, and the
 * slots after those two post-claim cell loads stay nops. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void lowerMirrorEveryFourth(
    void) {
  u8* record;
  u8* work;
  u8* cur;
  u8* obj;
  u8* next;
  u32 height;

  if ((D_80143E6C & 3) == 0) {
    record = queryFreeWorkSlot();
    workRecordCursor = record;
    if (record != 0) {
      claimRecordSeedPosition(record);
      work = D_1F800044;
      height = *(u32*)(work + 0x0C) - 0x10000u;
      cur = workRecordCursor;
      *(u32*)(work + 0x0C) = height;
      *(u32*)(cur + 0x1C) = height;
    }
  }
  obj = D_1F800044;
  if (--obj[9] == 0) {
    next = D_1F800044;
    next[1] = next[1] + 1;
  }
}

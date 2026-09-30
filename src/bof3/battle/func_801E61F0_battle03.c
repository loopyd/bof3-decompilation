#include "bof3/battle/battle03_internal.h"

/* @source 0x801E61F0
 * @behavior Counts down the current queued slot's timer byte +0x09; when that
 * byte expires it clears flag bit 0x80 of the 0x140-stride local work record
 * selected by the slot's work pointer byte +0x05, or of the 0x118-stride enemy
 * work record selected by that byte minus three for larger selectors, and then
 * clears the queued slot bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E61F0(void) {
  u8 selection;

  if (D_801EC2E0->unk_09-- == 0) {
    /* The published work pointer is the last word (+0x74) of the 0x78-byte queued
     * record; Battle03QueuedSlot's tail names compile four bytes late, so the word
     * is read by its evidenced offset, as the sibling +0x3c access does. */
    selection = ((Battle03LocalWork*)FIELD_REF(u32, D_801EC2E0, 0x74u))->unk_05;
    if (selection < 3u) {
      D_80145FB4[selection].flags_00 &= ~0x80u;
    } else {
      D_801EB630[selection - 3u].unk_100 &= ~0x80u;
    }
    clearQueuedSlotBytes();
  }
}

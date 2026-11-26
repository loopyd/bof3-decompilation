#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the record published at 0x1F800044.
 * Bound as a symbol in this target's map so the base re-materializes from the
 * cell exactly as the original does with `lui $v0,0x1F80` / `lw $v0,0x44($v0)`. */
extern u8* workRecordSlot; /* @source 0x1F800044 @kind data */

/* Shared overlay step-state byte seeded here with the published record's slot
 * code; the next entry of this overlay's handler pointer table (0x801F3790)
 * compares it against `0x10 << slot`. */
extern u8 D_80146867; /* @source 0x80146867 @kind data */

/* @source 0x801F3768
 * @behavior Seeds this overlay's shared step-state byte D_80146867 with the
 *           slot code `0x10 << n`, where n is the signed word at offset 0x18
 *           of the 0x98-byte work record published through the scratchpad
 *           pointer cell 0x1F800044. Takes no arguments, calls nothing and
 *           returns nothing: the whole body is one record read, the shift and
 *           the byte store. The next entry of the handler pointer table at
 *           0x801F5B6C/0x801F5B70, the already-lifted step at 0x801F3790,
 *           compares that same byte against `0x10` shifted left by the
 *           published record's slot index in the shared table at 0x80147A58,
 *           so this body arms the equality that step tests.
 *
 * Evidence: the payload span 0x801F3768-0x801F378F is 0x28 bytes and ten
 * instructions with no prologue and the single epilogue `jr $ra; nop`, so it
 * is one complete leaf function, not a fragment. The original loads the
 * scratchpad cell, takes `lw $v1,0x18($v0)`, builds the base with
 * `addiu $v0,$zero,0x10` plus `sllv $v0,$v0,$v1` and stores it with
 * `lui $at,%hi(D_80146867)` / `sb %lo(D_80146867)($at)`. No `jal` in the
 * payload names this span; the word 0x801F3768 appears once, at 0x801F5B6C,
 * inside the handler pointer run 0x801F5B58-0x801F5B84 that lists this
 * overlay's steps in address order, so the body is address-taken from that
 * table and its parameter roles stay structural.
 *
 * Shape: seeded from the original bytes. A payload scan of out/binaries for
 * the six-word identity 0x801F3768-0x801F377F found one hit, this payload at
 * offset 0xB68, and the eight-word identity up to the store found the same
 * single hit, so no byte-identical or near-identical sibling exists in either
 * direction and the clone lever does not apply. The declaration style, the
 * symbol-bound cell spelling and the metadata layout follow the sibling
 * body at 0x801F3130 of this target, which reads the same scratchpad cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishCodePublishedIndex(
    void) {
  D_80146867 = 0x10 << *(s32*)(workRecordSlot + 0x18);
}

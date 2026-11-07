#include "bof3/bof3.h"

/* @source 0x801F44A4 @kind unknown — cursor into the 8-entry, 0x60-byte-stride
 * record table that starts at 0x800E486C. The lifted sibling at 0x801F3870
 * republishes it as it walks the entries, the free-record finder at 0x801F3B98
 * reads byte +0x00 as the in-use flag through it, and this function caches the
 * table base in it. */
extern u8* D_801F44A4;

/* @source 0x801F3834
 * @behavior Publishes the base 0x800E486C of the 8-entry, 0x60-byte-stride
 *           record table as the shared cursor D_801F44A4 and then clears the
 *           in-use flag byte at the cursor, taking no arguments and returning
 *           nothing. The loop runs eight times but never advances the cursor,
 *           and it re-reads D_801F44A4 on every iteration, so the shipped code
 *           writes the flag byte of the first record eight times and leaves the
 *           cursor on the table base instead of walking the entries - the
 *           three uses of this table in the payload all end with the cursor
 *           either on the base (this function) or on a single entry (0x801F3870
 *           republishes base + 0x60 * i, 0x801F3B98 republishes the first entry
 *           whose flag byte is zero and returns it).
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the counter is an unsigned byte in a
 * bottom-tested loop (`andi`/`sltiu 8`), the same loop shape the exact sibling
 * clearWorkTableAt800E4800World02Area09813_801F5240 in emi/world02/area098/13
 * proves for an explicitly seeded `u8 i` walked with `i += 1u`, and the publish
 * of the base into the global cursor is the same `lui`/`ori` plus `sw` pair that
 * 0x801F3870 and 0x801F3B98 emit for the same literal. The body has no stack
 * frame, so these statements are the whole function. Its two in-payload callers
 * are the area initializer 0x801F2C48, which reaches it after priming the shared
 * work words at 0x800E4800..0x800E4823, and the 0x801F2D34 handler, which
 * reaches it when the countdown byte at +0x09 of the record published in
 * scratchpad cell 0x1F800044 expires and then reloads that countdown with 0x78,
 * so the reset runs on area entry and once per 120 ticks. 0x800E486C is main
 * RAM outside this payload's own 0x801F2C00 range; no other target in the tree
 * references the literal, so the eight-entry 0x60-byte record table has no
 * cross-payload naming evidence and the name stays anchored on the address.
 */
void resetRecordCursor800E486CAndClearCursorByteWorld02Area08013_801F3834(void) {
  u8 i;

  D_801F44A4 = PSX_PTR(u8, 0x800E486Cu);
  i = 0u;

  do {
    *D_801F44A4 = 0;
    i += 1u;
  } while (i < 8u);
}

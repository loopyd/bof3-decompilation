#include "bof3/bof3.h"

/* Scratchpad pointer cell holding the work record this overlay publishes. This
 * body reads the cell once per use - the record pointer does not stay live
 * across the three calls - so every access materialises the cell again.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Pointer run of this payload's textbin blob: the seventeen words from
 * 0x801F66D4 to 0x801F6714 all name the same 0x24-byte record run at
 * 0x801F66AC. This body passes the base of the run as the source-table argument
 * of the palette-slot allocator below and reads no word of it itself.
 * @source 0x801F66D4 @kind table
 */
extern u8* D_801F66D4[];

/* Helper of the shared main executable at 0x8014E4BC. The bytes prove only that
 * this body calls it once with the work record as its single argument; its role
 * is not established, so the callee keeps its address-anchored name.
 */
void func_8014E4BC(void* work);

/* Palette-slot allocator of the shared main executable at 0x80196CF0, the
 * address the etc/game/00 target maps as allocPaletteSlot and lifts exact: it
 * scans the eight slots of the palette block at 0x80145D94 for one whose flag
 * byte is clear, marks that slot occupied, stores the owner and source-table
 * pointers it is given, and answers the slot index, or 0xFF once every slot is
 * occupied. This body discards that answer.
 */
u8 func_80196CF0(u8* owner, u8* source_table);

/* Resource-request helper of the shared main executable at 0x8014D6B8: it
 * starts the asset id it is given, which is 9 here.
 */
void func_8014D6B8(u32 resource);

/* @source 0x801F3C8C
 * @behavior State handler of this overlay, reached through the word 0x801F6610,
 *           entry 0 of the dispatch table that publishes this payload's
 *           handlers, so no jal names it. It reads the work record published
 *           through the scratchpad cell 0x1F800044 and passes that record to the
 *           main-executable helper 0x8014E4BC, allocates one palette slot with
 *           the record as its owner and the payload's pointer run 0x801F66D4 as
 *           its source table, stores 1 into work byte 0x2A - the byte the
 *           resident pair-table helper 0x801F4A5C also writes, there with the
 *           second byte of a script record - and requests resource 9 through
 *           0x8014D6B8. Takes no arguments and returns nothing; the 0x18-byte
 *           frame only keeps $ra at 0x10 across the three calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void allocatePaletteSlotPayload(
    void) {
  func_8014E4BC(D_1F800044);
  func_80196CF0(D_1F800044, (u8*)D_801F66D4);
  D_1F800044[0x2A] = 1;
  func_8014D6B8(9);
}

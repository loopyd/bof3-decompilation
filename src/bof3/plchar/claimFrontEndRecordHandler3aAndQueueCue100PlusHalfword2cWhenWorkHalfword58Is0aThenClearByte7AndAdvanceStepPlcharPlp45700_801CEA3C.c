#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell before every access of the body, because a store
 * through the pointer may alias the cell itself, so the source keeps the
 * named-symbol form and evaluates it again at each access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8, allocated by the record allocator below. The two
 * bytes this body seeds keep their own address-derived symbol because the
 * original re-materialises the base and adds the scaled row offset for each
 * store: row byte 0 is the occupancy flag at 0x80143FC8 and row byte 5 is the
 * handler selector at 0x80143FCD.
 * @source 0x80143FC8 @kind table
 */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];

/* Record allocator of the shared EXE image, outside this payload: it answers
 * the index of the next free 0x74-byte row of the front-end record table, or
 * 0xFF when every row is occupied. The original stores the answer straight into
 * the work byte and only then compares that byte, so this overlay sees the
 * byte-wide return type.
 */
u8 func_8019601C(void);

/* Shared update helper of the shared EXE image, outside this payload. This
 * body calls it on every entry and discards any result. */
void func_8014D978(void);

/* Cue dispatcher of the shared EXE image, outside this payload: it takes the
 * cue id in a whole word, the call passing the work halfword 0x2C biased by
 * 0x100 without narrowing or masking it. */
void func_8015DF18(u32 cue_id);

/* @source 0x801CEA3C
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload: the body address appears once in the payload, as word
 *           0x801CF91C, index 7 of the sixteen-word handler run based at
 *           0x801CF900, so no branch of the payload names the span. The body
 *           runs the shared update helper func_8014D978 and returns while the
 *           work halfword 0x58 does not read 0x0A. When it does, the body
 *           claims a front-end record through the allocator func_8019601C,
 *           stores the answer in work byte 0x0B and, unless that byte reads
 *           0xFF, seeds the 0x74-byte row at 0x80143FC8 that the byte indexes:
 *           row byte 0 takes 1, the occupancy flag, and row byte 5 takes the
 *           handler selector 0x3A. Both stores rebuild the row offset from a
 *           fresh read of the work byte, so the whole multiply-by-0x74 chain
 *           runs twice, and the 0xFF test carries the first shift of the second
 *           chain in its delay slot. The body then hands the cue id 0x100 plus
 *           the work halfword 0x2C to the cue dispatcher func_8015DF18, clears
 *           the work byte 0x07 and increments the work step counter byte 0x02.
 *           It takes no arguments, returns nothing and answers no value; the
 *           0x18-byte frame only keeps $ra at 0x10 across the calls.
 *
 * Shape note: the 268-byte span is byte-identical over all 0x10C bytes, sha256
 * 228396289df61042dfb4a401bb6ea64ed785cc10668526fa1af8abed38ea77d2, to the
 * reviewed exact lift at 0x801CFD40 of the plp025 overlay, measured with one
 * payload comparison of the two spans. Every absolute word inside the span
 * resolves into the shared EXE image or is a lui offset, so no word moves with
 * the span and this is a true clone rather than a relocation-different sibling;
 * the plp025 source seeded the body below.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFrontEndRecordHandler3aAndQueueCue100PlusHalfword2cWhenWorkHalfword58Is0aThenClearByte7AndAdvanceStepPlcharPlp45700_801CEA3C(
    void) {
  func_8014D978();
  if (*(u16*)(D_1F800044 + 0x58) == 0x0A) {
    D_1F800044[0x0B] = func_8019601C();
    if (D_1F800044[0x0B] != 0xFFu) {
      D_80143FC8[(u32)D_1F800044[0x0B] * 0x74u] = 1;
      D_80143FCD[(u32)D_1F800044[0x0B] * 0x74u] = 0x3A;
    }
    func_8015DF18(*(u16*)(D_1F800044 + 0x2C) + 0x100);
    D_1F800044[0x07] = 0;
    D_1F800044[0x02] += 1;
  }
}

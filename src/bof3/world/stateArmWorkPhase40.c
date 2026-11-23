#include "bof3/bof3.h"

/* Scratchpad pointer cell that publishes the overlay work record. Declared
 * plain, not volatile: the second read of this body follows the store to the
 * record phase byte at +0x0A, and a byte store may alias the cell itself, so
 * the compiler re-materialises the cell address and reloads the pointer. The
 * same binding and the same reload point measured exact in the world04 area197
 * step at 0x801F3FC0. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Work-entity cursor of the shared main executable: it holds the address of
 * the selected entry of the entity table at 0x80146888. This body lowers the
 * unsigned halfword at +0x7E of that entry, whose role no reviewed record
 * establishes, so the offset stays anchored. */
extern u8* D_80146884; /* @source 0x80146884 @kind unknown */

/* @source 0x801F2C48
 * @behavior Overlay step handler, entry 0 of the local handler table at
 *   0x801F4C10 that the dispatcher at 0x801F2C04 reaches with the work record
 *   byte at +0x04. It loads the work record published at the scratchpad
 *   pointer cell 0x1F800044 and stores 0x40 in the record phase byte at +0x0A,
 *   reads that cell again and stores 1 in the record handler byte at +0x04 so
 *   the next dispatch reaches entry 1 at 0x801F2C88, then loads the work-entity
 *   cursor at 0x80146884 and lowers its unsigned halfword at +0x7E by 2. It
 *   takes no arguments, returns nothing and builds no frame; both immediates
 *   reach their stores through $v0, and the halfword store fills the return
 *   delay slot.
 *
 * Name evidence: the only word of this payload naming 0x801F2C48 is entry 0 of
 *   the handler table at 0x801F4C10, and the dispatcher at 0x801F2C04 indexes
 *   that table with the record byte at +0x04 (lbu, sll 2, addu on the table
 *   base, lw, jalr). The same-table step at 0x801F2C88 reads the byte at +0x0A
 *   as a phase counter, indexes the signed phase table at 0x801F4C18 with its
 *   low nibble and clears the byte at +0x04 once the counter reaches zero, so
 *   this body arms the pair: it seeds the phase count and selects handler 1.
 *   The halfword at +0x7E of the record published at 0x80146884 has no
 *   established role in the reviewed records of either cell, so the name keeps
 *   it offset-anchored.
 *
 * Shape evidence: a payload scan over every extracted binary under out/binaries
 *   found this 16-word body once, at file offset 0x48 of this target, so no
 *   byte-identical sibling exists anywhere and the clone lever is declined. The
 *   nearest structural siblings are the world04 area197 step at 0x801F3FC0 and
 *   the world02 area099 step at 0x801F2DEC: both carry this exact closing
 *   sequence (lhu +0x7E, load-delay nop, addiu -2 on that value, halfword store
 *   back) but reach it from a gated phase byte, and the world03 area132 body at
 *   0x801F2C48 clears a countdown and selects handler 1 through two separate
 *   reads of the same scratchpad cell. Those bodies are shape and idiom
 *   evidence only, not clone proof; every literal, offset and cell binding here
 *   comes from this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateArmWorkPhase40(
    void) {
  u8* work;
  u8* shared;

  work = D_1F800044;
  work[0x0a] = 0x40;
  work = D_1F800044;
  work[4] = 1;
  shared = D_80146884;
  *(u16*)(shared + 0x7e) -= 2;
}

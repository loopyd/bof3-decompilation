#include "bof3/bof3.h"

/* Scratchpad pointer cell that publishes the overlay work record. Declared
 * plain, not volatile: the second read of this body follows the store to the
 * record phase byte at +0x0A, and a byte store may alias the cell itself, so
 * the compiler re-materialises the cell address and reloads the pointer. The
 * same binding and the same reload point measured exact in this target's step
 * at 0x801F3FC0. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Work-entity cursor of the shared main executable: it holds the address of
 * the selected entry of the entity table at 0x80146888. This body lowers the
 * unsigned halfword at +0x7E of that entry, whose role no reviewed record
 * establishes, so the offset stays anchored. */
extern u8* D_80146884; /* @source 0x80146884 @kind unknown */

/* @source 0x801F3F80
 * @behavior Overlay step handler, entry 0 of the local handler table at
 *   0x801F4A80 that the dispatcher at 0x801F3F3C reaches with the work record
 *   byte at +0x04. It loads the work record published at the scratchpad
 *   pointer cell 0x1F800044 and stores 4 in the record phase byte at +0x0A,
 *   reads that cell again and stores 1 in the record handler byte at +0x04 so
 *   the next dispatch reaches entry 1 at 0x801F3FC0, then loads the work-entity
 *   cursor at 0x80146884 and lowers its unsigned halfword at +0x7E by 2. It
 *   takes no arguments, returns nothing and builds no frame; both immediates
 *   reach their stores through $v0, and the halfword store fills the return
 *   delay slot.
 *
 * Name evidence: the only word of this payload naming 0x801F3F80 is entry 0 of
 *   the handler table at 0x801F4A80, and the dispatcher at 0x801F3F3C indexes
 *   that table with the record byte at +0x04 (lbu, sll 2, addu on the table
 *   base, lw, jalr). The same-table step at 0x801F3FC0 reads the byte at +0x0A
 *   as a phase counter, indexes the signed phase table at 0x801F4A88 with its
 *   low nibble and clears the byte at +0x04 once the counter reaches zero, so
 *   this body arms the pair: it seeds the phase count and selects handler 1.
 *   The halfword at +0x7E of the record published at 0x80146884 has no
 *   established role in the reviewed records of either cell, so the name keeps
 *   it offset-anchored.
 *
 * Shape evidence: a payload scan over all 398 extracted binaries under
 *   out/binaries found this 16-word body once, at file offset 0x1380 of this
 *   target, so no byte-identical sibling exists and the clone lever is
 *   declined. A second pass that compares opcode and register fields alone
 *   matches six members of the family that differ in their immediates only.
 *   One of them is the world01 area039 handler at 0x801F2C48, whose own source
 *   records exact and whose 16 words differ from this span in exactly one,
 *   the stored phase literal, so it supplied the statement order, operand forms
 *   and declaration style; every literal, cell binding and offset here comes
 *   from this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armSelectLowerShared(
    void) {
  u8* work;
  u8* shared;

  work = D_1F800044;
  work[0x0a] = 4;
  work = D_1F800044;
  work[4] = 1;
  shared = D_80146884;
  *(u16*)(shared + 0x7e) -= 2;
}

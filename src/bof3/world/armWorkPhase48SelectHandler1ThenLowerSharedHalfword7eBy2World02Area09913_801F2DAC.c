#include "bof3/bof3.h"

/* Scratchpad pointer cell that publishes the overlay work record. Declared
 * plain, not volatile: the second read of this body follows the store to the
 * record phase byte at +0x0A, and a byte store may alias the cell itself, so
 * the compiler re-materialises the cell address and reloads the pointer. The
 * same binding and the same reload point measured byte-exact in the world01
 * area039 arm at 0x801F2C48, the near-identical sibling this body was
 * transcribed from. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Global pointer cell of the shared main executable: it publishes the record
 * whose unsigned halfword at +0x7E this body lowers. No reviewed record
 * establishes that field, so the offset stays anchored. */
extern u8* D_80146884; /* @source 0x80146884 @kind unknown */

/* @source 0x801F2DAC
 * @behavior Overlay step handler, entry 0 of the local handler table at
 *   0x801F4988 that the dispatcher at 0x801F2D68 reaches with the work record
 *   byte at +0x04. It loads the work record published at the scratchpad
 *   pointer cell 0x1F800044 and stores 0x48 in the record phase byte at +0x0A,
 *   reads that cell again and stores 1 in the record handler byte at +0x04 so
 *   the next dispatch reaches entry 1 at 0x801F2DEC, then loads the pointer
 *   cell at 0x80146884 and lowers the unsigned halfword at +0x7E of its record
 *   by 2. It takes no arguments, returns nothing and builds no frame; both
 *   immediates reach their stores through $v0, and the halfword store fills
 *   the return delay slot.
 *
 * Name evidence: the word at 0x801F4988 of this payload, entry 0 of the handler
 *   table the dispatcher at 0x801F2D68 indexes with record byte +0x04, is this
 *   address, and the same table holds 0x801F2DEC at entry 1, the step that
 *   counts that phase byte down and clears the byte at +0x04 once it reaches
 *   zero. This body therefore arms the pair: it seeds the phase count and
 *   selects handler 1. The halfword at +0x7E of the record published through
 *   0x80146884 has no established role, so that offset stays anchored.
 *
 * Shape evidence: the world01 area039 arm at 0x801F2C48 carries this body, and
 *   one 0x40-byte payload comparison of the two extracted spans differs in a
 *   single word, the immediate those two bodies store in the phase byte (0x48
 *   here, 0x40 there); every other word, including the load-delay nops and the
 *   halfword store in the return delay slot, is equal, so the spans are one
 *   body and not a clone pair. The statement order, the plain cell binding and
 *   the reload point come from that sibling; the literal stored here and the
 *   remainder of the site's words come from this span.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armWorkPhase48SelectHandler1ThenLowerSharedHalfword7eBy2World02Area09913_801F2DAC(
    void) {
  u8* work;
  u8* shared;

  work = D_1F800044;
  work[0x0a] = 0x48;
  work = D_1F800044;
  work[4] = 1;
  shared = D_80146884;
  *(u16*)(shared + 0x7e) -= 2;
}

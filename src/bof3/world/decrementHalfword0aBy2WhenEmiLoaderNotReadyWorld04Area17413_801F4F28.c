#include "bof3/bof3.h"

/* EXE-side EMI loader ready test: it answers nonzero only once the shared
 * loader reaches its ready state, and this body branches on a zero answer. The
 * same address is lifted exact as isEmiLoaderReady in src/bof3/io/, and the
 * handler neighbours of this table call it with no arguments. */
s32 func_80162D00(void); /* @source 0x80162D00 */

/* Record published through the global pointer cell at 0x80148210, outside this
 * payload. The original loads that pointer once and then reads and writes the
 * halfword at 0x0A through the loaded address, so the cell is declared at
 * halfword width and indexed at 5; the same-target exact handler 0x801F3D40
 * lowers the same halfword through the same declaration form. */
/* @source 0x80148210 @kind unknown */
extern u16* D_80148210;

/* @source 0x801F4F28
 * @behavior State handler of this overlay, reached through the word 0x801F6660,
 *           entry 20 of the dispatch table at 0x801F6610 that publishes this
 *           payload's handlers from inside the textbin blob at 0x801F4FAC, so no
 *           jal in the payload names it and no in-image caller supplies argument
 *           roles. It calls the EXE-side EMI loader ready test at 0x80162D00 with
 *           no arguments and acts on a zero answer only: while the loader is not
 *           ready it lowers the halfword at offset 0x0A of the record published
 *           through the global pointer cell 0x80148210 by 2, and a ready answer
 *           leaves the record untouched. It takes no arguments, returns nothing,
 *           and its 0x18-byte frame only keeps $ra at 0x10 across that call.
 *
 * The table order backs the role: entry 19 of the same table is the exact
 * handler 0x801F4D90, whose table-full arm lowers this same halfword by 2
 * through this same pointer cell, and the neighbouring entries are the overlay's
 * no-argument state handlers, so this body is the wait arm of that front-end
 * sequence. The gate byte itself keeps the shared callee's address, because this
 * target's map holds no row for 0x80162D00 and the generated undefined-function
 * file already binds the raw name.
 *
 * Shape notes from the original bytes: one pointer load feeds both the halfword
 * read and the halfword store, so the source indexes that declaration once
 * rather than keeping a record local; the halfword load delay slot and the jal
 * delay slot are the fill-in nops the original bytes carry. This 0x44-byte span
 * is not byte-identical to any payload under out/binaries - one scan of every
 * shipped overlay for the span answered no other hit - so no clone is claimed
 * and the same-target exact handler 0x801F3D40 supplies only the statement
 * spelling of the decrement, differing in its gate test.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword0aBy2WhenEmiLoaderNotReadyWorld04Area17413_801F4F28(void) {
  if (func_80162D00() == 0) {
    D_80148210[5] -= 2;
  }
}

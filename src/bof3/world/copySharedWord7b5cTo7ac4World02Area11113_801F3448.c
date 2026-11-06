#include "bof3/bof3.h"

extern u32 D_80147AC4; /* @source 0x80147AC4 @kind data */
extern u32 D_80147B5C; /* @source 0x80147B5C @kind data */

/* @source 0x801F3448
 * @behavior Copies the shared word at 0x80147B5C into the shared word at
 * 0x80147AC4. Each cell is read and written through its own lui/lw or
 * lui/sw pair, and the store sits in the return delay slot: that is the
 * plain multi-definition form of the two globals, so the original
 * translation unit declared no local for the value. Takes no arguments and
 * returns nothing.
 *
 * Name evidence: both cells live outside this overlay's payload in the
 * exe-resident state area, so the roles belong to shared game state rather
 * than to this overlay; the source and destination are anchored to their
 * addresses because no independent consumer in this target proves a
 * narrower role. Consumers elsewhere read 0x80147AC4 as a base indexed by
 * 0x98-byte strides and 0x80147B5C as the word at that base plus one stride,
 * which is a lead for a later naming pass, not proof for this body.
 *
 * Shape: no local is needed. The two-address copy has no byte-identical
 * sibling in any tracked payload, since the discriminating immediates are
 * this pair of cells, so it is seeded from the original bytes alone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copySharedWord7b5cTo7ac4World02Area11113_801F3448(void) {
  D_80147AC4 = D_80147B5C;
}

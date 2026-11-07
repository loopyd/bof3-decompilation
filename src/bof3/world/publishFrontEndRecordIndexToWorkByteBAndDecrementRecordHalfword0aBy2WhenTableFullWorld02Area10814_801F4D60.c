#include "bof3/bof3.h"

/* The overlay's work record is published through the scratchpad pointer cell at
 * 0x1F800044. This body reads the cell once per access group, so the cell is
 * bound as a named target-local symbol rather than through the constant-address
 * accessor: each group re-materialises `lui $vN,0x1F80` plus the folded
 * `lw $vN,68($vN)` pair the original bytes show, and the store through the
 * loaded record makes the second group reload the cell. The cell is a plain
 * pointer; no volatility is needed. */
extern u8* workRecord44World02Area10814; /* @source 0x1F800044 @kind data */

/* Record published through the global pointer cell at 0x80148210; this body
 * reads and writes back the halfword at its +0x0A offset through one base. The
 * same declaration and access spelling is exact in this target's sibling
 * decrementRecordHalfword0aUnlessMode5World02Area10814_801F4F0C. */
extern u16* D_80148210; /* @source 0x80148210 @kind unknown */

/* Front-end record allocator: returns the index of the next free 0x74-byte
 * record at 0x80143FC8, or 0xFF when the table is full. */
u8 func_8019601C(void);

/* @source 0x801F4D60
 * @behavior Stores the front-end record index returned by the shared allocator
 *           0x8019601C into the work byte at +0x0B of the record published at
 *           scratchpad pointer cell 0x1F800044, re-reading the cell for the
 *           store base, then re-reads that byte through a freshly loaded cell
 *           base and, only when it comes back 0xFF (the allocator's
 *           table-full result), subtracts 2 from the halfword at +0x0A of the
 *           record published through the global pointer cell 0x80148210. The
 *           single call keeps its `nop` delay slot, the 0x18-byte frame holds
 *           only $ra at 0x10, and the conditional arm keeps a `nop` in its
 *           branch delay slot while re-materialising the 0x80148210 base
 *           itself. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishFrontEndRecordIndexToWorkByteBAndDecrementRecordHalfword0aBy2WhenTableFullWorld02Area10814_801F4D60(void) {
  u8 index;

  index = func_8019601C();
  workRecord44World02Area10814[0xb] = index;
  if (workRecord44World02Area10814[0xb] == 0xff) {
    D_80148210[5] -= 2;
  }
}

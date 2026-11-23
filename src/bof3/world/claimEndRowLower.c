#include "bof3/bof3.h"

/* Front-end record table of the shared main-executable data segment: 20 rows of
 * 0x74 bytes based at 0x80143FC8, allocated by the record allocator below. This
 * body seeds three bytes of one row and the original re-materialises the base
 * and adds the scaled row offset separately for each store, so every field
 * keeps its own address-derived symbol: row byte 0 is the occupancy flag at
 * 0x80143FC8, row byte 5 the handler selector at 0x80143FCD and row byte 9 the
 * anchored byte at 0x80143FD1. */
extern u8 D_80143FC8[];
extern u8 D_80143FCD[];
extern u8 D_80143FD1[];

/* Work-entity cursor of the shared main executable: it holds the address of the
 * selected entry of the entity table at 0x80146888. The fallback arm lowers the
 * unsigned halfword at +0x7E of that entry, whose role no reviewed record
 * establishes, so the offset stays anchored. */
extern u8* D_80146884;

/* Record allocator of the shared main executable, outside this payload: it
 * answers the index of the next free 0x74-byte row of the front-end record
 * table, or 0xFF when every row is occupied. This body masks the answer to a
 * byte itself, so the byte-wide return type rides on the caller's own andi. */
u8 func_8019601C(void);

/* @source 0x801F2D54
 * @behavior Overlay record claim: asks the shared allocator func_8019601C for a
 * frame of the front-end record table at 0x80143FC8 and masks the answer to a
 * byte. When that index is not 0xFF it seeds the row: 1 in the occupancy byte at
 * 0x80143FC8, 0x36 in the handler selector at 0x80143FCD and 0 in the anchored
 * row byte at 0x80143FD1, all three reached with the index scaled by 0x74. When
 * the allocator answers 0xFF it takes the fallback arm instead and lowers the
 * unsigned halfword at +0x7E of the work-entity record published through the
 * global pointer cell 0x80146884 by 2. It takes no arguments and returns
 * nothing; the 0x18-byte frame exists only to keep the return address across the
 * allocator call.
 *
 * Name evidence: the only word of this payload that reaches this span is nothing
 * at all - no local table names it and no jal in the payload targets it - so the
 * name states the behavior its own bytes prove: the row it seeds carries the
 * handler selector 0x36 of the shared front-end record table, clears that row
 * byte at 0x80143FD1, and the arm taken when the table is full lowers the
 * anchored shared halfword at +0x7E by 2. The table base, its 0x74-byte stride,
 * the meaning of row byte 0 as occupancy and of row byte 5 as the handler
 * selector, and the allocator contract all come from the reviewed siblings
 * 0x801F2C04/0x801F2C48/0x801F2C88 of this target and from the lifted plchar
 * claim family that reads the same table.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimEndRowLower(
    void) {
  u8 index;

  index = func_8019601C();

  if (index != 0xffu) {
    D_80143FC8[(u32)index * 0x74u] = 1u;
    D_80143FCD[(u32)index * 0x74u] = 0x36u;
    D_80143FD1[(u32)index * 0x74u] = 0u;
  } else {
    u8* shared;

    shared = D_80146884;
    *(u16*)(shared + 0x7e) -= 2;
  }
}

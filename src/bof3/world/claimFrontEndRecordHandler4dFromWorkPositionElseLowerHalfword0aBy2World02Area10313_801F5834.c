#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Naming the cell keeps the original's
 * `lui $a0,0x1F80` plus `lw $a0,68($a0)` pair; the bare constant address lets
 * the compiler fold the cell into one saved register and load through a
 * different base.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Record published through the global pointer cell at 0x80148210. The body
 * lowers its halfword +0x0A by 2 when the front-end record table is full; the
 * same declaration and access spelling is exact in this target's sibling
 * advanceHalfword0aBy17WhenWorkByte79NonzeroWorld02Area10313_801F5738.
 * @source 0x80148210 @kind unknown
 */
extern u16* D_80148210;

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8. This body addresses every field through its own
 * absolute symbol, because the original re-materialises the base and adds the
 * scaled row offset for each access, so each field base stays separate.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* Row byte 5, the entry handler selector seeded with 0x4D; the handler table at
 * 0x801C7C70 documents +0x05 as that selector.
 * @source 0x80143FCD @kind unknown
 */
extern u8 D_80143FCD[];
/* Row word 0x34, the first word of the seeded position triple.
 * @source 0x80143FFC @kind unknown
 */
extern u8 D_80143FFC[];
/* Row word 0x38, the second word of the seeded position triple.
 * @source 0x80144000 @kind unknown
 */
extern u8 D_80144000[];
/* Row word 0x3C, the third word of the seeded position triple.
 * @source 0x80144004 @kind unknown
 */
extern u8 D_80144004[];

/* Front-end record allocator: returns the index of the next free 0x74-byte row
 * at 0x80143FC8, or 0xFF when the table is full. This overlay calls it once and
 * does not read the published scratchpad copy. */
u8 func_8019601C(void);

/* @source 0x801F5834
 * @behavior Claims one front-end record through the shared allocator
 *           func_8019601C and, unless the returned index is 0xFF, scales it by
 *           the 0x74-byte row stride and seeds the claimed row at 0x80143FC8:
 *           row byte +0 takes 1, the occupancy flag every seeder of this family
 *           writes, row byte +5 takes the handler selector 0x4D, and the word
 *           triple at row +0x34/+0x38/+0x3C takes the position words
 *           0x34/0x38/0x3C of the work object published through the scratchpad
 *           pointer cell 0x1F800044. A table-full answer (0xFF) instead lowers
 *           the halfword at +0x0A of the record published through the global
 *           pointer cell 0x80148210 by 2. The 0x18-byte frame only keeps $ra at
 *           0x10 across the allocator call. Takes no arguments, returns
 *           nothing. The role of handler selector 0x4D stays described by value
 *           and the -2 countdown step is not established, so the name reports
 *           the effect only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFrontEndRecordHandler4dFromWorkPositionElseLowerHalfword0aBy2World02Area10313_801F5834(
    void) {
  u8* work;
  u32 row;
  u8 index;

  index = func_8019601C();
  if (index != 0xFFu) {
    row = (u32)index * 0x74u;
    D_80143FC8[row] = 1;
    D_80143FCD[row] = 0x4D;
    work = D_1F800044;
    *(u32*)(D_80143FFC + row) = *(u32*)(work + 0x34);
    *(u32*)(D_80144000 + row) = *(u32*)(work + 0x38);
    *(u32*)(D_80144004 + row) = *(u32*)(work + 0x3C);
  } else {
    D_80148210[5] -= 2;
  }
}

#include "bof3/bof3.h"

/* 152-byte (0x98) work-record cell of the shared table at 0x80147A58. Only
 * the stride is proven here: the original scales the published slot code by
 * exactly 0x98, which is the same stride the resident record code uses. */
typedef struct World02Area11113WorkRecord {
  u8 bytes[0x98];
} World02Area11113WorkRecord;

/* Shared scroll pair of the exe-resident world state: the halfword at
 * 0x80145EC6 and the halfword four bytes later at 0x80145ECA, both read here
 * with `lh` so they are signed halfwords. */
extern s16 D_80145EC6[]; /* @source 0x80145EC6 @kind data */
extern s16 D_80145ECA; /* @source 0x80145ECA @kind data */

/* The three shared mode bytes the overlay's entry states seed: 0x801448EB is
 * the state selector, 0x801448EC and 0x801448ED are its two companion bytes. */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */
extern u8 D_801448EC; /* @source 0x801448EC @kind data */
extern u8 D_801448ED; /* @source 0x801448ED @kind data */

/* Packed cell-code map inside this overlay's trailing data blob: two nibbles
 * per byte, four bytes per map column, so the original addresses it as
 * column * 4 + row / 2. */
extern const u8 D_801F5C18[]; /* @source 0x801F5C18 @kind data */

/* Base of the shared 0x98-byte work-record table. */
extern World02Area11113WorkRecord D_80147A58[]; /* @source 0x80147A58 @kind bss */

/* Shared front-end startup helper, called last by the overlay's entry states. */
void func_8015C088(void);

/* @source 0x801F4510
 * @behavior Area entry that turns the shared scroll pair into a packed-map
 *           cell and selects the work record for that cell. It halves the
 *           signed halfword at 0x80145EC6 minus 0xB and the signed halfword at
 *           0x80145ECA minus 0xB1, then reads the nibble at
 *           0x801F5C18[col * 4 + ((row & 0xFF) >> 1)]: the low nibble when the
 *           row index is odd and the high nibble when it is even. That nibble
 *           minus one is the record slot, published to 0x801448ED while
 *           0x801448EB takes the state selector 0x11. The argument is
 *           published to 0x801448EC and to byte 8 of the 0x98-byte work record
 *           at that slot of the shared table 0x80147A58, and the body ends by
 *           calling the shared startup helper at 0x8015C088. Takes the
 *           selector code in $a0 and returns nothing.
 *
 * Name evidence: 0x80145EC6/0x80145ECA are the shared scroll pair the world
 *           overlays read (area01613 reads the same pair through
 *           func_80166CB0), the offset-8 byte of the 0x80147A58 records is the
 *           selector byte the step code at 0x801F35F4 publishes and 0x801F4140
 *           reads back masked with 0x7, and 0x801448EB is the state selector
 *           the dispatcher indexes. The argument has no separately proven role,
 *           so it stays structural in the parameter list.
 *
 * Shape: this payload holds no byte-identical twin and no clone-eligible
 *           sibling anywhere in the tree (the 0xB1/0xB grid origin immediates
 *           appear only in this overlay), so the body is seeded from the
 *           original bytes. The nearest sibling is the reader step at
 *           0x801F35F4, whose first forty instructions are the same lookup:
 *           the same `lh` pair, the same subtract-then-halve forms, the same
 *           odd-row branch with the low nibble out of line behind `j` and the
 *           even-row arm falling through behind `srl 4`. Its tail differs (it
 *           only reads record byte 8 and stores the masked selector at
 *           0x80143F50). The step at 0x801F4140 in this same payload is the
 *           caller and passes 3, 7, 5 and 1.
 *
 *           Two measured facts pin the operand forms. The column halving is
 *           the compiler's logical shift only when the quotient is kept in a
 *           u8 local (a signed `/ 2` in an s32 local emits `sra`, which this
 *           compiler produces for every other lifted `/ 2` in the tree), and
 *           the row halving stays `sra`, so the two coordinates are declared
 *           apart exactly that way. The record store is bound at
 *           0x80147A58 + 8 through the 0x98-byte record type, which reproduces
 *           the original `lui`/`addu`/`sb` pair on that address.
 * @status partial
 * @match 62.30
 * @residual 38/61 instructions, 236 of 244 bytes; first difference at
 *           0x801F4548. The original copies the row quotient into a second
 *           register (`move a2,v0`) before the odd/even test, then recomputes
 *           the row byte offset from that copy inside each arm, which leaves
 *           0x801448EB's neighbouring byte mask in the branch delay slot. This
 *           candidate shares the offset computation ahead of the test instead.
 *           Every tested spelling that computes the offset inside the arms
 *           (a copied `line`, a per-arm u8 half, the offset written inline, and
 *           a u8 or s32 column) moves the first `lui`/`lh` of the scroll pair
 *           into $v0 and cascades; only a pre-test computation keeps the
 *           original's $v1/$v0 homes. Remaining class: register allocation and
 *           branch-delay-slot fill, not statement order or constants.
 */
void selectWorkRecordSlotFromScrollCellAndEnterState11World02Area11113_801F4510(
    u8 value) {
  const u8* cell;
  u8 col;
  s32 row;
  s32 half;
  u8 code;
  u8 slot;

  col = (D_80145ECA - 0xB1) / 2;
  row = (D_80145EC6[0] - 0xB) / 2;
  half = (row & 0xFF) >> 1;
  if (row & 1) {
    cell = (const u8*)D_801F5C18 + (col & 0xFF) * 4;
    code = cell[half] & 0xF;
  } else {
    cell = (const u8*)D_801F5C18 + (col & 0xFF) * 4;
    code = cell[half] >> 4;
  }
  slot = code - 1;
  D_801448ED = slot;
  D_801448EB = 0x11;
  D_801448EC = value;
  D_80147A58[slot].bytes[8] = value;
  func_8015C088();
}

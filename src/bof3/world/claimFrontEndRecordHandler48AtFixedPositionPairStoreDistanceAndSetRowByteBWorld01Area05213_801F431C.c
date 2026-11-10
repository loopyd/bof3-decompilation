#include "bof3/bof3.h"

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8. The allocator below answers the index of the next
 * free row, or 0xFF when every row is occupied. */
extern u8 D_80143FC8[];

/* Shared front-end record allocator: it answers the index of the next free
 * 0x74-byte row at 0x80143FC8, or 0xFF when the table is full. */
s32 func_8019601C(void);

/* Shared distance helper: it takes the 16.16 word pair (x, y) and answers
 * their distance as a word. */
s32 func_8015477C(s32 x, s32 y);

/* @source 0x801F431C
 * @behavior Claims one front-end record and seeds it at a fixed position. The
 *           body asks the shared allocator func_8019601C for a row index,
 *           masks it to a byte and, unless that byte is 0xFF, scales it by the
 *           0x74-byte row stride and seeds the row at 0x80143FC8: row byte 0
 *           takes 1, the occupancy flag, row word 0x34 takes the fixed first
 *           coordinate 0x890000, row byte 5 takes the handler selector 0x48,
 *           row word 0x38 takes the fixed second coordinate 0x118000, row word
 *           0x3C takes the shared distance answer for that pair scaled left by
 *           eight, and row byte 0x0B takes 0x10. Nothing is returned and a full
 *           table leaves every row untouched. Its 0x28-byte frame keeps $ra and
 *           $s0 across the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Evidence: the payload span 0x801F431C-0x801F43AF is one complete function of
 * 0x94 bytes with a single `jr $ra` epilogue, so it is neither data nor a
 * fragment. The store sequence and the compiler's multiply-by-0x74 chain come
 * from the bytes; the neighbouring asm span func_801F4290 is the same body one
 * literal set away (handler 0x48, byte 0x0B = 8, coordinates 0x880000 and
 * 0x110000) and supplied the statement order. No other payload of this tree
 * holds these 0x94 bytes, so the span has no byte-identical twin.
 *
 * Matching note: the statements alone compile to a 0x18-byte frame, and the
 * original reserves 0x28 bytes, whose spills sit at 0x20 and 0x24 with nothing
 * below reading or writing the extra 0x10. That space comes from a declaration
 * the original translation unit carried that emitted no code, so
 * `frame_reserve` restores the size and nothing else; only its size is
 * constrained, and its name and type are a reconstruction choice flagged for
 * review.
 */
void claimFrontEndRecordHandler48AtFixedPositionPairStoreDistanceAndSetRowByteBWorld01Area05213_801F431C(
    void) {
  u8* record;
  s32 index;
  u8 frame_reserve[0x10]; /* size-only frame reservation, see the note above */

  index = func_8019601C() & 0xFF;
  if (index != 0xFF) {
    record = (u8*)(D_80143FC8 + index * 0x74);
    record[0] = 1;
    *(s32*)(record + 0x34) = 0x890000;
    record[5] = 0x48;
    *(s32*)(record + 0x38) = 0x118000;
    *(s32*)(record + 0x3C) =
        func_8015477C(*(s32*)(record + 0x34), 0x118000) << 8;
    record[0xB] = 0x10;
  }
}

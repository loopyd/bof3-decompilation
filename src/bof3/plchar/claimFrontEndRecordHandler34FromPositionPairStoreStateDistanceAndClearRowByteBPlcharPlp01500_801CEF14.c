#include "bof3/bof3.h"

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8. This body addresses every field through its own
 * absolute symbol, because the original re-materialises the base and adds the
 * scaled row offset for each access, so each field base stays separate.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* Row byte 1, the entry state byte the family selection tests compare with 1.
 * @source 0x80143FC9 @kind unknown
 */
extern u8 D_80143FC9[];
/* Row byte 5, the entry handler selector.
 * @source 0x80143FCD @kind unknown
 */
extern u8 D_80143FCD[];
/* Row byte 0x0B, cleared by this body after the row is seeded.
 * @source 0x80143FD3 @kind unknown
 */
extern u8 D_80143FD3[];
/* Row word 0x34, the first coordinate of the seeded position pair.
 * @source 0x80143FFC @kind unknown
 */
extern u8 D_80143FFC[];
/* Row word 0x38, the second coordinate of the seeded position pair.
 * @source 0x80144000 @kind unknown
 */
extern u8 D_80144000[];
/* Row halfword 0x3E, the distance halfword the gate helpers read as a signed
 * reference.
 * @source 0x80144006 @kind unknown
 */
extern u8 D_80144006[];

/* Front-end record allocator of the shared main-executable data segment: it
 * answers the index of the next free 0x74-byte row at 0x80143FC8, or 0xFF when
 * every row is occupied. */
u8 func_8019601C(void);

/* Distance helper of the shared main executable: it takes a signed 16.16 word
 * pair as its two coordinates and answers their distance as a halfword. The
 * original adds 0x100 to the answer with a plain 32-bit add and no widening or
 * masking, so this overlay sees a word return.
 * @source 0x8015477C
 */
extern s32 func_8015477C(s32 x, s32 y);

/* @source 0x801CEF14
 * @behavior Claims one free front-end record and seeds it from a caller-supplied
 *           position pair. The body asks the shared allocator func_8019601C for
 *           a row index, masks it to a byte and, unless that byte is 0xFF,
 *           scales it by the 0x74-byte row stride and seeds the row at
 *           0x80143FC8: row byte 0 takes 1, the occupancy flag, row byte 5 takes
 *           the handler selector 0x34, row byte 1 takes the caller's state
 *           byte, row word 0x34 takes the first signed coordinate shifted left
 *           16 and row word 0x38 takes the second the same way, so the pair
 *           lands in signed 16.16 form. It then re-reads row word 0x34 and
 *           passes it with the shifted second coordinate to the shared distance
 *           helper func_8015477C, storing that answer plus 0x100 in row
 *           halfword 0x3E, and finally clears row byte 0x0B. Nothing is
 *           returned and a full table leaves every row untouched. The three
 *           in-image call sites are func_801CED8C, func_801CEE08 and
 *           func_801CEE44, all inside this payload's func_801CED1C: the first
 *           and third pass state 0 with unsigned argument copies, the second
 *           passes state 1 with the sign-extended halves of its two arguments,
 *           and each ignores the result.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the row offset is kept in one saved
 * register built by the compiler's multiply-by-0x74 chain (sll 3, subu, sll 2,
 * addu, sll 2), whose first shift the compiler places in the delay slot of the
 * 0xFF test. The distance call re-reads row word 0x34 from the table instead of
 * reusing the stored value, and the shifted second coordinate is computed once
 * for both the row store and the call. The four byte stores follow the row
 * offset order, so the entry state byte lands after row byte 5 and row word
 * 0x34, which keeps the state byte in $s1, the first coordinate in $s2 and the
 * second in $s3 as the original does. The 228-byte body is byte-identical to the
 * exact source of the same function in plp27a/00
 * (claimFrontEndRecordHandler34FromPositionPairStoreStateDistanceAndClearRowByteBPlcharPlp27a00_801CF640),
 * compared by payload bytes rather than read from its metadata.
 */
void claimFrontEndRecordHandler34FromPositionPairStoreStateDistanceAndClearRowByteBPlcharPlp01500_801CEF14(
    u8 state, s16 x, s16 y) {
  u32 row;
  u8 index;

  index = func_8019601C();
  if (index != 0xFFu) {
    row = (u32)index * 0x74u;
    D_80143FC8[row] = 1;
    D_80143FC9[row] = state;
    D_80143FCD[row] = 0x34;
    *(s32*)(D_80143FFC + row) = x << 16;
    *(s32*)(D_80144000 + row) = y << 16;
    *(s16*)(D_80144006 + row) =
        func_8015477C(*(s32*)(D_80143FFC + row), y << 16) + 0x100;
    D_80143FD3[row] = 0;
  }
}

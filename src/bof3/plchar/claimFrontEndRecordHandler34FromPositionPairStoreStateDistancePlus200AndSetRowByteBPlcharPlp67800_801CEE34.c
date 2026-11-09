#include "bof3/bof3.h"

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8. This body addresses every field through its own
 * absolute symbol, because the original re-materialises the base and adds the
 * scaled row offset for each access, so each field base stays separate.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* Row byte 1, the entry state byte the family selection tests compare with 1;
 * this body seeds it from its first argument.
 * @source 0x80143FC9 @kind unknown
 */
extern u8 D_80143FC9[];
/* Row byte 5, the entry handler selector seeded with 0x34.
 * @source 0x80143FCD @kind unknown
 */
extern u8 D_80143FCD[];
/* Row byte 0x0B, written 1 after the row is seeded. The other claim bodies of
 * this family clear it instead, so its role is not established.
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
 * every row is occupied. The original reaches it before it has touched any
 * argument register, so the incoming $a0-$a2 still travel through this call;
 * the no-argument form is kept for that reason.
 */
u8 func_8019601C(void);

/* Distance helper of the shared main executable: it takes a signed 16.16 word
 * pair as its two coordinates and answers their distance as a halfword. The
 * original adds 0x200 to the answer with a plain 32-bit add and no widening or
 * masking, so this overlay sees a word return.
 * @source 0x8015477C
 */
extern s32 func_8015477C(s32 x, s32 y);

/* @source 0x801CEE34
 * @behavior Claims one free front-end record and seeds it from a
 *           caller-supplied position pair. The body asks the shared allocator
 *           func_8019601C for a row index, masks it to a byte and, unless that
 *           byte is 0xFF, scales it by the 0x74-byte row stride and seeds the
 *           row at 0x80143FC8: row byte 5 takes the handler selector 0x34, row
 *           word 0x34 takes the first signed coordinate shifted left 16, row
 *           byte 0 takes the occupancy flag 1, row byte 1 takes the caller
 *           state byte and row word 0x38 takes the second coordinate the same
 *           way, so the pair lands in signed 16.16 form. It then re-reads row
 *           word 0x34 and passes it with the shifted second coordinate to the
 *           shared distance helper func_8015477C, storing that answer plus
 *           0x200 in row halfword 0x3E, and finally writes 1 into row byte
 *           0x0B. Nothing is returned and a full table leaves every row
 *           untouched. The five in-image call sites are 0x801CEC90,
 *           0x801CECB8, 0x801CECE8, 0x801CED18 and 0x801CEDB4, all inside the
 *           still-unlifted map cell claim handler func_801CEC28 of this
 *           payload: they pass entry states 0, 4, 0, 3 and 2 with the same
 *           narrowed halfword pair in $a1 and $a2 and a byte in $a3 that this
 *           body never reads, so only its first three argument registers are
 *           named. The roles of row byte 0x0B, of the plus 0x200 bias and of
 *           the caller state byte are not established, so the name and this
 *           block describe the effect only.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the row offset is kept in one saved
 * register built by the compiler's multiply-by-0x74 chain (sll 3, subu, sll 2,
 * addu, sll 2), whose first shift the compiler places in the delay slot of the
 * 0xFF test. The literal 1 is shared by the occupancy byte and the trailing row
 * byte 0x0B and survives the distance call in its own saved register, so the
 * body needs five saved registers ($s0 row, $s1 the constant, $s2 the state
 * byte, $s3 the first coordinate, $s4 the second). The distance call re-reads
 * row word 0x34 from the table instead of reusing the stored value, and the
 * shifted second coordinate is computed once for both the row store and the
 * call. The seven stores land in the row order handler byte 5, word 0x34,
 * occupancy byte 0, state byte 1, word 0x38, halfword 0x3E, byte 0x0B. The
 * 236-byte span was compared by payload bytes against the exact plp349/00 lift
 * of the same body at 0x801CEE24 (md5 of the equal spans
 * ac3382afc2fe4699cf3a8fcfb8757135, zero differing bytes) and an own clone
 * dry-run reported the two ranges byte-identical at 236 bytes, so the statement
 * order, operand forms and declaration style were taken from that lift with
 * this function's own name. That body had no caller in its payload; this one
 * has the five jal sites above, which is the only difference in what the
 * metadata blocks report.
 */
void claimFrontEndRecordHandler34FromPositionPairStoreStateDistancePlus200AndSetRowByteBPlcharPlp67800_801CEE34(
    u8 state, s16 x, s16 y) {
  u32 row;
  u8 index;

  index = func_8019601C();
  if (index != 0xFFu) {
    row = (u32)index * 0x74u;
    D_80143FC8[row] = 1;
    D_80143FCD[row] = 0x34;
    D_80143FC9[row] = state;
    *(s32*)(D_80143FFC + row) = x << 16;
    *(s32*)(D_80144000 + row) = y << 16;
    *(s16*)(D_80144006 + row) =
        func_8015477C(*(s32*)(D_80143FFC + row), y << 16) + 0x200;
    D_80143FD3[row] = 1;
  }
}

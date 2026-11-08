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
 * every row is occupied. Splat binds its address through this target's
 * generated undefined_funcs_auto.txt.
 */
u8 func_8019601C(void);

/* Distance helper of the shared main executable: it takes a signed 16.16 word
 * pair as its two coordinates and answers their distance as a halfword. The
 * original adds 0x200 to the answer with a plain 32-bit add and no widening or
 * masking, so this overlay sees a word return. Splat binds its address through
 * this target's generated undefined_funcs_auto.txt.
 */
extern s32 func_8015477C(s32 x, s32 y);

/* @source 0x801CF62C
 * @behavior Claims one free front-end record and seeds it from a
 *           caller-supplied position pair. Five jal call sites inside
 *           func_801CF420 of this payload reach it, at 0x801CF488 and
 *           0x801CF4E0 with a zero state byte, at 0x801CF4B0 with state 4, at
 *           0x801CF510 with state 3 and at 0x801CF5AC with state 2; each passes
 *           the sign-extended halves of that body's own first two arguments as
 *           the position pair, in $a1 and $a2, and also fills $a3 with a masked
 *           byte of func_80166CB0's result that this body never reads, so the
 *           caller's fourth argument has no role here and the parameter list
 *           stays three wide. No word of this payload holds 0x801CF62C, so no
 *           code-pointer run names it, and every call site ignores the result.
 *           The body asks the shared allocator func_8019601C for a row index,
 *           masks it to a byte and, unless that byte is 0xFF, scales it by the
 *           0x74-byte row stride and seeds the row at 0x80143FC8: row byte 0
 *           takes 1, the occupancy flag, row byte 5 takes the handler selector
 *           0x34, row byte 1 takes the caller state byte, row word 0x34 takes
 *           the first signed coordinate shifted left 16 and row word 0x38 takes
 *           the second the same way, so the pair lands in signed 16.16 form. It
 *           then re-reads row word 0x34 and passes it with the shifted second
 *           coordinate to the shared distance helper func_8015477C, storing
 *           that answer plus 0x200 in row halfword 0x3E, and finally writes 1
 *           into row byte 0x0B. Nothing is returned and a full table leaves
 *           every row untouched. The roles of row byte 0x0B, of the +0x200 bias
 *           and of the state codes are not established, so the name and this
 *           block describe the effect only.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the row offset is kept in one saved
 * register built by the compiler multiply-by-0x74 chain (sll 3, subu, sll 2,
 * addu, sll 2), whose first shift the compiler places in the delay slot of the
 * 0xFF test. The literal 1 is shared by the occupancy byte and the trailing row
 * byte 0x0B and survives the distance call in its own saved register, so the
 * body needs five saved registers ($s0 row, $s1 the constant, $s2 the state
 * byte, $s3 the first coordinate, $s4 the second). The state-byte store
 * follows row byte 5 and precedes the two coordinate words, which is what keeps
 * the state byte in $s2 and the first coordinate in $s3. The distance call
 * re-reads row word 0x34 from the table instead of reusing the stored value,
 * and the shifted second coordinate is computed once for both its row store and
 * the call. The 236-byte span measured byte-identical, relocations included, to
 * the payload spans of the exact plp056/00 lift at 0x801D0158, plp016/00 at
 * 0x801CFF30 and plp026/00 at 0x801D0354 (one payload cmp of the four spans,
 * md5 ac3382afc2fe4699cf3a8fcfb8757135), so this statement order and operand
 * spelling are that body copied with this overlay's own declarations.
 */
void claimFrontEndRecordHandler34FromPositionPairStoreStateDistancePlus200AndSetRowByteBPlcharPlp26700_801CF62C(
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

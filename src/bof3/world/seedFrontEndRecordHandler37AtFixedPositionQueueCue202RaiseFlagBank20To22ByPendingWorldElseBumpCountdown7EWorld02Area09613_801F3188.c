#include "bof3/bof3.h"

/* Front-end record table of the shared main executable: 20 rows of 0x74 bytes
 * based at 0x80143FC8. The original re-materialises the `%hi` half of each
 * field's own symbol and adds the scaled row offset in place, so this body
 * declares every field base it writes separately and indexes each with one
 * shared offset. Only the written fields are declared.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* Row byte 0x05, the handler selector of this record family; the shared
 * RecordSlot view of this table in include/bof3/ui/game00_internal.h documents
 * the same byte and src/bof3/ui/func_8019A0E4.c dispatches through it. This
 * body seeds the handler 0x37 seeder's selector value.
 * @source 0x80143FCD @kind unknown
 */
extern u8 D_80143FCD[];
/* Row byte 0x06, cleared by this body.
 * @source 0x80143FCE @kind unknown
 */
extern u8 D_80143FCE[];
/* Row byte 0x07, seeded with 0x12.
 * @source 0x80143FCF @kind unknown
 */
extern u8 D_80143FCF[];
/* Row byte 0x09, seeded with 1.
 * @source 0x80143FD1 @kind unknown
 */
extern u8 D_80143FD1[];
/* Row byte 0x29, seeded with 6.
 * @source 0x80143FF1 @kind unknown
 */
extern u8 D_80143FF1[];
/* Row halfword 0x2C, seeded with 0x58; its role is not established in this
 * payload, so it stays described by offset and value.
 * @source 0x80143FF4 @kind unknown
 */
extern u8 D_80143FF4[];
/* Row word 0x34, the first word of the position triple this body seeds with a
 * fixed value rather than from the work object the handler 0x37 seeder of
 * world01 area043 reads.
 * @source 0x80143FFC @kind unknown
 */
extern u8 D_80143FFC[];
/* Row word 0x38, the second word of the same fixed position triple.
 * @source 0x80144000 @kind unknown
 */
extern u8 D_80144000[];
/* Row word 0x3C, the third word of the same fixed position triple.
 * @source 0x80144004 @kind unknown
 */
extern u8 D_80144004[];

/* Halfword of the shared main executable holding the pending world id that the
 * overlay trampolines select on; this body compares it with 0x60.
 * @source 0x80143F00 @kind data
 */
extern u16 D_80143F00;
/* Signed scenario-state byte of the shared main executable; this body tests it
 * against 7.
 * @source 0x80146870 @kind data
 */
extern s8 D_80146870;
/* Work-record pointer cell of the shared main executable. The records published
 * through it are 0x140 bytes long and their halfword at +0x7E is the countdown
 * the neighbouring world lifts read at that offset.
 * @source 0x80146884 @kind data
 */
extern u8* D_80146884;
/* Main-RAM flag bank of the shared main executable; this body raises one of its
 * bits 0x20, 0x21 or 0x22.
 * @source 0x80144F28 @kind data
 */
extern u8 D_80144F28[];

/* Front-end record allocator of the shared main executable: answers the index
 * of the next free 0x74-byte row of the table at 0x80143FC8, or 0xFF when every
 * row is occupied. Its byte answer is why the original narrows the result with
 * one `andi 0xFF`.
 */
extern u8 func_8019601C(void);

/* Sound-cue dispatcher of the shared main executable; it takes a cue id and
 * discards any result. */
extern void func_8015DF18(u32 cue);

/* Flag-bank setter of the shared main executable: raises one bit of the bank it
 * is given. */
extern void func_8015B580(void* flag_bank, u8 bit_index);

/* @source 0x801F3188
 * @behavior Handler of this overlay's step table: it claims one front-end
 *           record through the shared allocator func_8019601C and keeps the
 *           returned index in a byte local. A free index seeds the claimed
 *           0x74-byte row of the table at 0x80143FC8 through one shared
 *           offset - the occupancy 1 at +0x00, the handler selector 0x37 at
 *           +0x05, the fixed position triple 0x410000/0x3C8000/0x2000000 at
 *           +0x34/+0x38/+0x3C, the halfword 0x58 at +0x2C, then 0x12 at +0x07,
 *           1 at +0x09, 0 at +0x06 and 6 at +0x29, in that store order - then
 *           submits sound cue 0x202 through func_8015DF18 and raises one bit of
 *           the main-RAM flag bank 0x80144F28: bit 0x20 when the pending world
 *           id 0x80143F00 reads 0x60, otherwise bit 0x21 while the signed
 *           scenario-state byte 0x80146870 reads below 7 and bit 0x22 from 7
 *           up.
 *           A full table (index 0xFF) seeds nothing and instead subtracts 2
 *           from the countdown halfword at +0x7E of the 0x140-byte work record
 *           published by the pointer cell 0x80146884. Takes no arguments and
 *           returns nothing; the 0x18-byte frame keeps only $ra, and the
 *           allocator index is narrowed once at the assignment rather than
 *           re-read from the scratchpad, which is why the offset chain runs
 *           once and every row store adds it to its own field symbol.
 *
 * Matching shape: the occupancy store leads the sequence in the source, which
 * is what keeps the constant 1 in $a1 across the later +0x09 store and lets it
 * materialise above the first field store. A seed carrying the same stores in
 * the emitted order instead, with the occupancy store sixth, first-differed in
 * the register of the 0x3C8000 pair and left the +0x38 store after the +0x09
 * one.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedFrontEndRecordHandler37AtFixedPositionQueueCue202RaiseFlagBank20To22ByPendingWorldElseBumpCountdown7EWorld02Area09613_801F3188(
    void) {
  u8 index;
  u32 offset;
  u8* record;

  index = func_8019601C();
  if (index != 0xffu) {
    offset = (u32)index * 0x74u;
    D_80143FC8[offset] = 1u;
    D_80143FCD[offset] = 0x37u;
    *(u32*)(D_80143FFC + offset) = 0x410000;
    *(u32*)(D_80144000 + offset) = 0x3C8000;
    *(u32*)(D_80144004 + offset) = 0x2000000;
    *(u16*)(D_80143FF4 + offset) = 0x58u;
    D_80143FCF[offset] = 0x12u;
    D_80143FD1[offset] = 1u;
    D_80143FCE[offset] = 0u;
    D_80143FF1[offset] = 6u;

    func_8015DF18(0x202u);

    if (D_80143F00 == 0x60) {
      func_8015B580(D_80144F28, 0x20);
    } else if (D_80146870 < 7) {
      func_8015B580(D_80144F28, 0x21);
    } else {
      func_8015B580(D_80144F28, 0x22);
    }
  } else {
    record = D_80146884;
    *(u16*)(record + 0x7E) -= 2;
  }
}

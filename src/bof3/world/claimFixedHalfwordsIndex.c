#include "bof3/bof3.h"

/* Front-end record table of the shared main executable: 0x74-byte rows based at
 * 0x80143FC8. The original re-materialises the `%hi` half of each field's own
 * symbol and adds the scaled row offset in place, so this body declares every
 * field base it writes separately and indexes each with one shared offset.
 * Only the written fields are declared.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* Row byte 0x05, the handler selector of this record family; the same byte is
 * the dispatch index of src/bof3/ui/func_8019A0E4.c and the sibling seeds at
 * src/bof3/world/seedFrontEndRecordHandler37AtFixedPositionQueueCue202RaiseFlagBank20To22ByPendingWorldElseBumpCountdown7EWorld02Area09613_801F3188.c
 * write other selector values here. This body seeds 0x92.
 * @source 0x80143FCD @kind unknown
 */
extern u8 D_80143FCD[];
/* Row byte 0x29, seeded with 6. Its role is not established in this payload,
 * so it stays described by offset and value.
 * @source 0x80143FF1 @kind unknown
 */
extern u8 D_80143FF1[];
/* Row halfword 0x2E, seeded with 0. The handler 0x37 seeder of world02 area096
 * copies this halfword from the work record at the same offset instead.
 * @source 0x80143FF6 @kind unknown
 */
extern u16 D_80143FF6[];
/* Row halfword 0x30, seeded with -0x1E; the second halfword of the pair whose
 * role is not established in this payload.
 * @source 0x80143FF8 @kind unknown
 */
extern u8 D_80143FF8[];
/* Row byte 0x0B, the index of the current work record; the RecordSlot view in
 * include/bof3/world/area03213_internal.h documents the same byte of the same
 * table, and the sibling lifts at src/bof3/plchar/ clear it after a claim.
 * @source 0x80143FD3 @kind unknown
 */
extern u8 D_80143FD3[];
/* Work-record pointer cell of the shared main executable: 0x80146884 publishes
 * the pointer to the current one of the 0x98-byte records at 0x80146888.
 * @source 0x80146884 @kind data
 */
extern u8* D_80146884;
/* Shared work-record table of the shared main executable, 0x98 bytes per
 * record. This body uses the base only, as the anchor of the byte difference
 * the pointer cell above is measured against.
 * @source 0x80146888 @kind data
 */
extern u8 D_80146888[];

/* Front-end record allocator of the shared main executable: answers the index
 * of the next free 0x74-byte row of the table at 0x80143FC8, or 0xFF when every
 * row is occupied. Its byte answer is why the original narrows the result with
 * one `andi 0xFF`. */
u8 func_8019601C(void);

/* @source 0x801F4050
 * @behavior Claims one free front-end record through the shared allocator
 *           func_8019601C and seeds it with fixed values. The allocator answers
 *           the row index as a byte, and this body keeps that answer in one byte
 *           local: 0xFF means every row is occupied and leaves the table
 *           untouched, and any other value seeds the 0x74-byte row it selects
 *           at 0x80143FC8, in this store order - the occupancy 1 at row byte
 *           0x00, the handler selector 0x92 at row byte 0x05, the signed
 *           halfword -0x1E at row halfword 0x30, 6 at row byte 0x29, zero at
 *           row halfword 0x2E, and the index of the current work record at row
 *           byte 0x0B.
 *           That last value is the slot of the 0x98-byte work record the
 *           pointer cell 0x80146884 holds, measured as a byte difference
 *           against the table base 0x80146888: the original multiplies the
 *           difference by 0x1B, 0x1FF and 0x40001 - the expansion of the
 *           reciprocal of the stride, since 0x98 * 0x1B * 0x1FF * 0x40001 is
 *           2^39 - 8 - negates the wrapped product and shifts it right by three
 *           with a sign-filling shift, which answers the slot for every slot of
 *           the table (measured over slots 0..35).
 *           Nothing is returned. The payload holds no call to this address, and
 *           the code-pointer run that starts at 0x801F49C0 publishes it as the
 *           word at 0x801F49E0, so the body is reached through that table and no
 *           in-image caller supplies argument roles: it takes none and uses
 *           only its 0x18-byte $ra frame.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Representation evidence: the byte-array cast-and-dereference spelling of
 * the zero halfword store measured 50/53 instructions at 212 bytes, with its
 * address/store block ahead of the pointer-cell load. The owning GCC 2.7.2
 * scheduler dump gave that load a true-dependence edge on the raw halfword
 * store. Declaring this field base as a halfword array and indexing it by
 * offset / sizeof(u16) measured 53/53 at 212 bytes without changing store
 * order. The row stride 0x74 is even, so this is the same address and access
 * width. GCC's sched.c exempts varying non-byte array accesses from a fixed
 * cell's memory dependence; this form lets the cell load precede the store.
 *
 * Shape notes from the original bytes: the first 0xC4 bytes of this span are
 * byte-identical to the head of func_801F3650 of emi/world04/area171/13, proven
 * by a byte comparison of the two payloads under out/binaries; that span is
 * still on its `asm` row, so its disassembly - the original bytes - supplied the
 * statement order, the operand forms and the offset spelling, and this span's
 * own epilogue supplies the return. The row offset is built by the compiler's
 * multiply-by-0x74 chain (sll 3, subu, sll 2, addu, sll 2) on the byte local,
 * whose first shift the compiler places in the delay slot of the 0xFF test. That
 * offset is one named u32 local and stays in $a0; a second named local for the
 * pointer difference or the pointer itself moves it to $a1 in every measured
 * shape, which is why the difference stays inline in the last store. The slot
 * chain reuses $v0 and $v1 as the pointer difference leaves $v0.
 */
void claimFixedHalfwordsIndex(
    void) {
  u8 index;
  u32 offset;

  index = func_8019601C();
  if (index != 0xffu) {
    offset = (u32)index * 0x74u;
    D_80143FC8[offset] = 1u;
    D_80143FCD[offset] = 0x92u;
    *(s16*)(D_80143FF8 + offset) = -0x1e;
    D_80143FF1[offset] = 6u;
    D_80143FF6[offset / sizeof(u16)] = 0u;
    D_80143FD3[offset] =
        (u8)(-(s32)(((u32)D_80146884 - (u32)D_80146888) * 27 * 511 * 0x40001) >>
             3);
  }
}

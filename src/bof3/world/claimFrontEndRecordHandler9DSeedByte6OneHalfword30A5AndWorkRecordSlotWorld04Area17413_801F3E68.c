#include "bof3/bof3.h"

/* Front-end record table of the shared main executable: 0x74-byte rows based at
 * 0x80143FC8. The original re-materialises the `%hi` half of each field's own
 * symbol and adds the scaled row offset in place, so this body declares every
 * field base it writes separately and indexes each with one shared offset.
 * Only the written fields are declared.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* Row byte 0x05, the handler selector of this record family; this body seeds
 * 0x9D.
 * @source 0x80143FCD @kind unknown
 */
extern u8 D_80143FCD[];
/* Row byte 0x06, seeded with 1. Its role is not established in this payload, so
 * it stays described by offset and value.
 * @source 0x80143FCE @kind unknown
 */
extern u8 D_80143FCE[];
/* Row halfword 0x30, seeded with 0xA5. Its role is not established in this
 * payload; the scenario seeders copy the same halfword from work offset 0x30.
 * @source 0x80143FF8 @kind unknown
 */
extern u8 D_80143FF8[];
/* Row halfword 0x2E, seeded with 0x18 times the work-record slot plus 0xC. Its
 * role is not established in this payload; the scenario seeders copy the same
 * halfword from work offset 0x2E.
 * @source 0x80143FF6 @kind unknown
 */
extern u8 D_80143FF6[];
/* Row byte 0x0B, the index of the current work record; the sibling lifts of this
 * table clear or store the same byte.
 * @source 0x80143FD3 @kind unknown
 */
extern u8 D_80143FD3[];
/* Work-record pointer cell of the shared main executable: 0x80146884 publishes
 * the pointer to the current one of the 0x98-byte records at 0x80146888.
 * @source 0x80146884 @kind data
 */
extern u8* D_80146884;
/* Shared work-record table of the shared main executable, 0x98 bytes per
 * record. This body uses the base only, as the anchor of the byte difference the
 * pointer cell above is measured against.
 * @source 0x80146888 @kind data
 */
extern u8 D_80146888[];

/* Front-end record allocator of the shared main executable: answers the index of
 * the next free 0x74-byte row of the table at 0x80143FC8, or 0xFF when every row
 * is occupied. Its byte answer is why the original narrows the result with one
 * `andi 0xFF`. */
u8 func_8019601C(void);

/* @source 0x801F3E68
 * @behavior Claims one free front-end record through the shared allocator
 *           func_8019601C and seeds it with fixed values plus two fields derived
 *           from the current work record. The allocator answers the row index as
 *           a byte, and this body keeps that answer in one byte local: 0xFF
 *           means every row is occupied and leaves the table untouched, and any
 *           other value seeds the 0x74-byte row it selects at 0x80143FC8, in
 *           this store order - the occupancy 1 at row byte 0x00, the handler
 *           selector 0x9D at row byte 0x05, 1 at row byte 0x06 and 0xA5 at row
 *           halfword 0x30, then the two derived fields: the slot index of the
 *           current work record at row byte 0x0B and, at row halfword 0x2E,
 *           0x18 times that slot plus 0xC.
 *           That slot is the byte difference between the pointer cell 0x80146884
 *           and the 0x98-byte work-record table base 0x80146888 scaled by the
 *           stride: the original multiplies the difference by 0x1B, 0x1FF and
 *           0x40001 - the expansion of the reciprocal of the stride, since 0x98 *
 *           0x1B * 0x1FF * 0x40001 is 2^39 - 8 - negates the wrapped product and
 *           shifts it right by three with a sign-filling shift, which answers the
 *           slot for every slot of the table.
 *           Nothing is returned. The payload holds no jal to this address; the
 *           overlay dispatch table at 0x801F6610 publishes it as its entry 4, the
 *           word 0x801F6620, so the body is reached through that table and no
 *           in-image caller supplies argument roles: it takes none and uses only
 *           its 0x18-byte $ra frame.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the row offset is built by the compiler's
 * multiply-by-0x74 chain (sll 3, subu, sll 2, addu, sll 2) on the byte local,
 * whose first shift the compiler places in the delay slot of the 0xFF test. That
 * offset is one named u32 local and stays in $a0; the pointer difference stays
 * inline in its store. The slot is one s32 local: the row byte store truncates
 * it with the `sb` itself and its second use re-masks it with `andi 0xFF`.
 *
 * Representation evidence: a u8 slot local measured 50/57 with the sign-filling
 * shift replaced by a logical one, because the byte truncation lets the compiler
 * narrow the shift; the s32 local keeps `negu` plus `sra`. Placing the slot
 * computation after the row halfword 0x30 store measured 51/57 at 228 bytes,
 * because the scheduler then hoisted the pointer-cell load and the 0x9D store
 * past the occupancy store; computing the slot before that halfword store
 * measured 57/57 at 228 bytes. Spelling all six stores against the single base
 * symbol D_80143FC8 with constant field offsets measured 51/57, the same first
 * difference, so the split field declarations cost nothing.
 *
 * The statement order, operand forms and offset spelling come from the exact
 * sibling of another target,
 * src/bof3/world/claimFrontEndRecordHandler92SeedFixedHalfwordsAndWorkRecordIndexWorld04Area19713_801F4050,
 * which seeds the same table through the same allocator; the two spans are not
 * byte-identical (228 bytes here against 212 there, different stored literals and
 * this body's derived row halfword 0x2E), so it supplied shape only.
 */
void claimFrontEndRecordHandler9DSeedByte6OneHalfword30A5AndWorkRecordSlotWorld04Area17413_801F3E68(
    void) {
  u8 index;
  u32 offset;
  s32 slot;

  index = func_8019601C();
  if (index != 0xffu) {
    offset = (u32)index * 0x74u;
    D_80143FC8[offset] = 1u;
    D_80143FCD[offset] = 0x9du;
    D_80143FCE[offset] = 1u;
    slot = -(s32)(((u32)D_80146884 - (u32)D_80146888) * 27 * 511 * 0x40001) >> 3;
    *(u16*)(D_80143FF8 + offset) = 0xa5u;
    D_80143FD3[offset] = slot;
    *(u16*)(D_80143FF6 + offset) = (u8)slot * 0x18 + 0xc;
  }
}

#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. This body re-reads the cell through its
 * own address load for every store, and indexing each field separately is the
 * form the original bytes show, so no local copy is kept.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;
/* Pointer cell of the shared record whose halfword at 0x0A the full-table arm
 * consumes two units of.
 * @source 0x80148210 @kind unknown
 */
extern u16* D_80148210;
/* Front-end record table of the shared main executable: 0x74-byte rows based at
 * 0x80143FC8. The original re-materialises the `%hi` half of each field's own
 * symbol and adds the scaled row offset in place, so this body declares every
 * field base it writes separately and indexes each with its own read of the work
 * object. Only the written fields are declared.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* Row byte 0x05, the handler selector of this record family; this body seeds
 * 0xA3.
 * @source 0x80143FCD @kind unknown
 */
extern u8 D_80143FCD[];
/* Row byte 0x06, seeded with 1. Its role is not established in this payload, so
 * it stays described by offset and value.
 * @source 0x80143FCE @kind unknown
 */
extern u8 D_80143FCE[];
/* Row byte 0x07, cleared to 0 here. Its role is not established in this
 * payload.
 * @source 0x80143FCF @kind unknown
 */
extern u8 D_80143FCF[];
/* Row byte 0x0B, seeded with 1 here; the sibling lifts of this table store the
 * current work record's slot in the same byte.
 * @source 0x80143FD3 @kind unknown
 */
extern u8 D_80143FD3[];
/* Row word 0x4C, which this body fills with the address of the current work
 * record. Nothing else in this lane's evidence reads the field back, so it stays
 * anchored to its offset.
 * @source 0x80144014 @kind unknown
 */
extern u8 D_80144014[];

/* Front-end record allocator of the shared main executable: answers the index of
 * the next free 0x74-byte row of the table at 0x80143FC8, or 0xFF when every row
 * is occupied. */
u8 func_8019601C(void);

/* @source 0x801F4D90
 * @behavior Claims one free front-end record through the shared allocator
 *           func_8019601C, publishes that answer as work byte 0x0B of the work
 *           object the scratchpad cell 0x1F800044 points at, and then re-reads
 *           that byte. Any value other than 0xFF seeds the 0x74-byte row the
 *           work byte selects at 0x80143FC8, in this store order: occupancy 1 at
 *           row byte 0x00, the handler selector 0xA3 at row byte 0x05, 1 at row
 *           byte 0x06, 0 at row byte 0x07, 1 at row byte 0x0B, and the address of
 *           the work object itself at row word 0x4C. When the work byte is 0xFF,
 *           which is the allocator's table-full answer, the table is left
 *           untouched and the arm instead consumes two units of the halfword at
 *           0x0A of the record published through the shared pointer cell
 *           0x80148210.
 *           Nothing is returned, and the 0x18-byte frame only keeps $ra at 0x10
 *           across the allocator call. The payload holds no jal to this address;
 *           its only in-image reference is the code-pointer word 0x801F665C,
 *           entry 19 of the overlay dispatch table at 0x801F6610, so the body is
 *           reached through that table and no in-image caller supplies argument
 *           roles: it takes none.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: every row store re-derives its offset
 * from a fresh load of the work byte, because each store to the record table may
 * alias the pointer cell, so the source indexes with the inline expression rather
 * than a saved offset local. The multiply-by-0x74 chain is the compiler's
 * sll 3 / subu / sll 2 / addu / sll 2 expansion, and its first shift lands in the
 * delay slot of the 0xFF test. The constant 1 stays in one register across the
 * three stores that write it.
 *
 * Statement order, operand forms and offset spelling come from the exact
 * same-target sibling that claims and seeds the same table through the same
 * allocator,
 * src/bof3/plchar/adjustEvenWorkByte8ClaimFrontEndRecordHandler1BSelectSequenceAndQueueCueThenAdvanceWorkByte2PlcharPlp47800_801CF4CC.c,
 * and the full-table arm from the same-target
 * decrementScratchWorkLevelTripleBy4OrSetFlag40World04Area17413_801F4C68. This
 * 408-byte span is not byte-identical to any payload under out/binaries: one
 * search of every shipped overlay for the span answered no hit, so those two
 * supplied shape only and no clone is claimed.
 */
void claimFrontEndRecordHandlerA3SeedByte6Byte7WorkSlotAndWorkPointerElseConsumeHalfword0aBy2World04Area17413_801F4D90(
    void) {
  D_1F800044[0xb] = func_8019601C();

  if (D_1F800044[0xb] != 0xffu) {
    D_80143FC8[(u32)D_1F800044[0xb] * 0x74u] = 1u;
    D_80143FCD[(u32)D_1F800044[0xb] * 0x74u] = 0xa3u;
    D_80143FCE[(u32)D_1F800044[0xb] * 0x74u] = 1u;
    D_80143FCF[(u32)D_1F800044[0xb] * 0x74u] = 0u;
    D_80143FD3[(u32)D_1F800044[0xb] * 0x74u] = 1u;
    *(u32*)(D_80144014 + (u32)D_1F800044[0xb] * 0x74u) = (u32)D_1F800044;
  } else {
    D_80148210[5] -= 2;
  }
}

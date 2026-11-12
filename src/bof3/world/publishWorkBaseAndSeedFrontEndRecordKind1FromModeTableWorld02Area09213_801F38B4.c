#include "bof3/bof3.h"

/* The shared work base this overlay publishes as the scratchpad cursor: the
 * front-end record allocator writes the claimed record index into its byte
 * 0x0B, and the record's halfwords 0x2E/0x30 are copied from it.
 * @source 0x80146110 @kind unknown
 */
extern u8 D_80146110[];
/* Scratchpad work-object pointer cell; the original re-materialises its
 * address for every access that follows a byte store, so it keeps the symbol
 * form.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;
/* Front-end record table and the field bases of its 0x74-byte rows: the
 * original re-materialises `lui at,0x8014` for every store, so each field is
 * addressed as its own absolute symbol rather than through one struct base.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];
/* @source 0x80143FCE @kind unknown */
extern u8 D_80143FCE[];
/* @source 0x80143FD4 @kind unknown */
extern u8 D_80143FD4[];
/* @source 0x80143FD8 @kind unknown */
extern u8 D_80143FD8[];
/* @source 0x80143FF6 @kind unknown */
extern u8 D_80143FF6[];
/* @source 0x80143FF8 @kind unknown */
extern u8 D_80143FF8[];
/* Shared selector byte: its value is the index this overlay reads its mode
 * table with.
 * @source 0x80144F5C @kind unknown
 */
extern u8 D_80144F5C;
/* Overlay-local mode table: the payload's D_801F3A88 textbin run, eight bytes
 * in, supplies the byte this seeder copies into record word 0x10. The generated
 * undefined_syms_auto row names the address, and the same payload's asm rows at
 * 0x801F355C and 0x801F3708 read it at this base.
 * @source 0x801F3A90 @kind table
 */
extern u8 D_801F3A90[];

/* Front-end record allocator: returns the index of the next free 0x74-byte
 * record at 0x80143FC8, or 0xFF when the table is full. */
u8 func_8019601C(void);

/* @source 0x801F38B4
 * @behavior Publishes the shared work base D_80146110 as the scratchpad work
 *           cursor at 0x1F800044, stores the front-end record index returned by
 *           func_8019601C into that base's byte 0x0B and, unless that byte reads
 *           0xFF, seeds the 0x74-byte record it selects at 0x80143FC8: record
 *           byte 0x00 = 1, byte 0x05 = the handler selector 6, byte 0x06 = the
 *           record kind 1, word 0x0C = 0, word 0x10 = the byte
 *           D_801F3A90[D_80144F5C] and the halfwords at 0x2E/0x30 copied from
 *           the work base at the same offsets. A full record table leaves the
 *           table untouched. Takes no arguments and returns nothing, and the
 *           overlay code-pointer run at 0x801F413C reaches it as its index 10
 *           step handler, so no jal in this payload names it.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the record writes re-read work byte 0x0B
 * after every byte store, so the first three seed statements address the
 * scratchpad cursor cell directly. The last four seed statements share one
 * cursor load in $a1, which the source keeps as a single local pointer, and only
 * byte 0x0B is re-read through it. The record kind is the literal 1 the record
 * byte 0x00 store already materialises in $a0, so both byte stores reuse that
 * one register. Statement order, operand forms, offset spelling and declaration
 * style are taken from the exact sibling
 * publishWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld01Area03813_801F2E60
 * at 0x801F2E60, whose body is the same stream plus an entry guard on a
 * scratchpad selector byte that this span does not have. The shape is also fixed
 * inside this payload by three asm rows whose decoded streams equal this one
 * word for word except at carried symbols: 0x801F355C and 0x801F3708 differ only
 * in the work base (0x80145E90 and 0x80145FD0) and the selector byte (0x80144F5A
 * and 0x80144F5B), and 0x801F33AC differs in kind 3 and the mode-table base
 * 0x801F3A88. Every one of those spans carries a different immediate or symbol,
 * and this span's mode table is payload-local, so no byte-identical twin exists
 * in any target and the clone lever does not apply.
 */
void publishWorkBaseAndSeedFrontEndRecordKind1FromModeTableWorld02Area09213_801F38B4(
    void) {
  u8* work;

  D_1F800044 = D_80146110;
  D_1F800044[0x0B] = func_8019601C();

  if (D_1F800044[0x0B] != 0xFFu) {
    D_80143FC8[(u32)D_1F800044[0x0B] * 0x74u] = 1u;
    D_80143FCD[(u32)D_1F800044[0x0B] * 0x74u] = 6u;
    D_80143FCE[(u32)D_1F800044[0x0B] * 0x74u] = 1u;

    work = D_1F800044;
    *(u32*)(D_80143FD4 + (u32)work[0x0B] * 0x74u) = 0u;
    *(u32*)(D_80143FD8 + (u32)work[0x0B] * 0x74u) = D_801F3A90[D_80144F5C];
    *(u16*)(D_80143FF6 + (u32)work[0x0B] * 0x74u) = *(u16*)(work + 0x2E);
    *(u16*)(D_80143FF8 + (u32)work[0x0B] * 0x74u) = *(u16*)(work + 0x30);
  }
}

#include "bof3/bof3.h"

/* Scratchpad work objects: byte 0 of the scratchpad holds the last claimed
 * front-end record index (every seeder in this family stores it and re-reads
 * it), and the cell at 0x1F800044 holds the current work object whose words
 * 0x34/0x38/0x3C are the position triple. */
extern u8 D_1F800000;  /* @source 0x1F800000 @kind unknown */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Front-end record table at 0x80143FC8 - 0x74 bytes per record (the original
 * scales the allocator index by 0x74). Only the fields this body writes are
 * declared. */
extern u8 D_80143FC8[]; /* @source 0x80143FC8 @kind bss */
extern u8 D_80143FCD[]; /* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCE[]; /* @source 0x80143FCE @kind unknown */
extern u8 D_80143FD1[]; /* @source 0x80143FD1 @kind unknown */
extern u8 D_80143FF4[]; /* @source 0x80143FF4 @kind unknown */
extern u8 D_80143FFC[]; /* @source 0x80143FFC @kind unknown */
extern u8 D_80144000[]; /* @source 0x80144000 @kind unknown */
extern u8 D_80144004[]; /* @source 0x80144004 @kind unknown */

/* Front-end record allocator: returns the index of the next free 0x74-byte
 * record at 0x80143FC8, or 0xFF when the table is full. */
u8 func_8019601C(void);

/* @source 0x801F2E28
 * @behavior Overlay spawn step: claims one front-end record through the shared
 * allocator func_8019601C, publishes the returned index in scratchpad byte
 * 0x1F800000 and, unless that index is 0xFF, seeds the claimed 0x74-byte row at
 * 0x80143FC8: byte +0 takes 1 (the occupancy flag the whole family sets), byte
 * +5 takes the handler selector 0x37 (the same +5 selector byte the front-end
 * entry table documents), the halfword at +0x2C takes 0x64, byte +9 takes 1,
 * byte +6 takes 0, and the word triple at +0x34/+0x38/+0x3C takes the current
 * work object's position words 0x34/0x38/0x3C, with the constant 0x800000
 * added to the +0x3C word. Takes no arguments and returns nothing; a full
 * table (index 0xFF) leaves every row untouched.
 *
 * Evidence: the published index serves two purposes in the original, which is
 * why both spellings appear. The first row store and the 0xFF test use the
 * allocator result itself, while every later row address re-reads the
 * scratchpad byte (`lbu` of 0x1F800000 before each `(index * 0x74)` shift
 * chain), so the source keeps the local index for the first store and the
 * scratchpad byte view for the rest - the same split the exact sibling
 * func_801F92DC takes. The 0x74 stride is the compiler's shift form
 * `sll 3 / subu / sll 2 / addu / sll 2`, i.e. `(u32)index * 0x74u`. The three
 * position words are read from one load of the 0x1F800044 cell (named cell
 * symbol, not the scratchpad accessor, which folds to the same `lui 0x1F80`
 * plus `lw 0x44(reg)` pair the original uses) and the 0x800000 constant is
 * materialised in $a0 and added with `addu`. Neither the +0x2C halfword nor the
 * 0x800000 bias has a proven role in this payload, so both stay described by
 * offset and value.
 *
 * Name evidence: record byte 0x05 is the entry table's handler selector. The
 * shared RecordSlot view in include/bof3/ui/game00_internal.h documents 0x05 as
 * the handler selector for the table at 0x801C7C70, and
 * src/bof3/ui/func_8019A0E4.c dispatches that byte through D_801C7C70. The
 * family's record kind is byte 0x06 instead: the shared seeder at 0x8015CB18
 * stores byte 0x05 = 6 and byte 0x06 = its kind argument, and the world02
 * seeders at 0x801F5AAC, 0x801F5B2C and 0x801F5C00 pass that kind. This body
 * writes 0x37 to byte 0x05 and 0 to byte 0x06, so the name carries the handler
 * selector rather than a kind.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedFrontEndRecordHandler37FromWorkPositionWorld01Area04313_801F2E28(
    void) {
  u8* scratch;
  u8* work;
  u8 index;

  index = func_8019601C();
  scratch = &D_1F800000;
  *scratch = index;

  if (index != 0xffu) {
    D_80143FC8[(u32)index * 0x74u] = 1u;
    D_80143FCD[(u32)*scratch * 0x74u] = 0x37u;
    *(u16*)(D_80143FF4 + (u32)*scratch * 0x74u) = 0x64u;
    D_80143FD1[(u32)*scratch * 0x74u] = 1u;
    D_80143FCE[(u32)*scratch * 0x74u] = 0u;

    work = D_1F800044;
    *(u32*)(D_80143FFC + (u32)*scratch * 0x74u) = *(u32*)(work + 0x34);
    *(u32*)(D_80144000 + (u32)*scratch * 0x74u) = *(u32*)(work + 0x38);
    *(u32*)(D_80144004 + (u32)*scratch * 0x74u) =
        *(u32*)(work + 0x3C) + 0x800000u;
  }
}

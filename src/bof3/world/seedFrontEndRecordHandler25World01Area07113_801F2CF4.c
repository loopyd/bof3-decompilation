#include "bof3/bof3.h"

/* Scratchpad byte 0 holds the last claimed front-end record index: every
 * seeder in this family stores it there and later row addresses re-read it. */
extern u8 D_1F800000; /* @source 0x1F800000 @kind unknown */

/* Front-end record table at 0x80143FC8 - 0x74 bytes per record (the original
 * scales the allocator index by 0x74). Only the two bytes this body writes are
 * declared. */
extern u8 D_80143FC8[]; /* @source 0x80143FC8 @kind bss */
extern u8 D_80143FCD[]; /* @source 0x80143FCD @kind unknown */

/* Front-end record allocator: returns the index of the next free 0x74-byte
 * record at 0x80143FC8, or 0xFF when the table is full. */
u8 func_8019601C(void);

/* @source 0x801F2CF4
 * @behavior Overlay spawn step: claims one front-end record through the shared
 * allocator func_8019601C, publishes the returned index in scratchpad byte
 * 0x1F800000 and, unless that index is 0xFF, seeds the claimed 0x74-byte row at
 * 0x80143FC8: byte +0 takes 1 (the occupancy flag the whole family sets) and
 * byte +5 takes the handler selector 0x25. Takes no arguments and returns
 * nothing; a full table (index 0xFF) leaves every row untouched.
 *
 * Evidence: the span is a complete function - addiu $sp,-0x18 with
 * sw $ra,0x10($sp), one jal to func_8019601C, and the epilogue
 * lw $ra,0x10($sp); addiu $sp,$sp,0x18; jr $ra; nop - with a single 0x88-byte
 * body and one conditional arm. The published index serves two purposes, which
 * is why both spellings appear: the 0xFF test and the first row store use the
 * allocator result itself (`andi $v1,$v0,0xFF` before `sb $v0,0x0($a0)` where
 * $a0 is lui/addiu of D_1F800000), while the second row address re-reads the
 * scratchpad byte (`lbu $v1,0x0($a0)` with its load-delay nop) before repeating
 * the `(index * 0x74)` shift chain `sll 3 / subu / sll 2 / addu / sll 2`. Both
 * row stores re-materialise `lui $at,0x8014` and add the scaled index into $at
 * (`sb $v1,%lo(D_80143FC8)($at)` and `sb $v1,%lo(D_80143FCD)($at)`), so each
 * store is its own extern base with the index added, not a computed record
 * base. The stored literals are 1 and 0x25.
 *
 * Name evidence: record byte 0x05 is the front-end entry table's handler
 * selector - the shared RecordSlot view in include/bof3/ui/game00_internal.h
 * documents 0x05 as the handler selector for the table at 0x801C7C70, and
 * src/bof3/ui/func_8019A0E4.c dispatches that byte through D_801C7C70 - so this
 * body writes handler selector 0x25. Byte +0 is the occupancy flag the whole
 * family sets. The statement order, the index/scratchpad split and the row
 * store spelling come from the family's exact siblings
 * seedFrontEndRecordHandler37FromWorkPositionWorld01Area04313_801F2E28 and
 * seedFrontEndRecordHandler1CFromWorkPositionWorld00Area01013_801F2CDC, whose
 * leading claim-and-publish block is byte-identical to this body's; those
 * siblings additionally copy the work object's position words and this one does
 * not, so the name carries the handler selector alone. The value 0x25 has no
 * consumer inside this payload, so it stays described by value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedFrontEndRecordHandler25World01Area07113_801F2CF4(void) {
  u8* scratch;
  u8 index;

  index = func_8019601C();
  scratch = &D_1F800000;
  *scratch = index;

  if (index != 0xffu) {
    D_80143FC8[(u32)index * 0x74u] = 1u;
    D_80143FCD[(u32)*scratch * 0x74u] = 0x25u;
  }
}

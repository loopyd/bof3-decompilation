#include "bof3/bof3.h"

/* Front-end record table at 0x80143FC8: the shared allocator hands out indices
 * into it and every record row is 0x74 bytes (the original scales the index by
 * 0x74). Only the occupancy byte at +0, the selector byte at +5 and the
 * position pair at +0x34/+0x38 are proven by this target's body. */
extern u8 D_80143FC8[]; /* @source 0x80143FC8 @kind bss */

/* Scratch work object published through the pointer cell at 0x1F800044. The
 * original materialises the cell with `lui 0x1F80` plus `lw 0x44(reg)` and
 * keeps the loaded pointer for both position words, so the named cell symbol -
 * not the scratchpad accessor, which folds to the same pair - is the binding,
 * and one pointer local serves the pair. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Front-end record allocator: returns the index of the next free 0x74-byte
 * record at 0x80143FC8, or 0xFF when the table is full. */
u8 func_8019601C(void);

/* @source 0x801F2CDC
 * @behavior Seeds one claimed front-end record from the current work position:
 * it claims the record through the shared allocator func_8019601C and, unless
 * the returned index is 0xFF, seeds the claimed 0x74-byte row at 0x80143FC8 -
 * byte +0 takes 1 (the occupancy flag the whole family sets) and byte +5 takes
 * the handler selector 0x1C - then copies the current work object's position
 * pair: the row's words at +0x34 and +0x38 take words 0x34 and 0x38 of the
 * scratch work object published at 0x1F800044. Takes no arguments and returns
 * nothing; a full table (index 0xFF) leaves every row untouched.
 *
 * Evidence: payload asm at 0x801F2CDC - addiu $sp,-0x18 and sw $ra,0x10($sp),
 * then jal func_8019601C, andi $v1,$v0,0xFF and the 0xFF compare in $v0 with
 * `sll $v0,$v1,3` in the branch delay slot; the index scale is the compiler's
 * 0x74 form `sll 3 / subu / sll 2 / addu / sll 2` on the allocator result, then
 * lui/addiu of D_80143FC8 into $v1 and `addu $v0,$v0,$v1`, so all four stores
 * address one computed row base ($v0): sb 1 at 0x0, sb 0x1C at 0x5, and the
 * word pair - lui $a0,(0x1F800044 >> 16) plus lw $a0,(0x1F800044 & 0xFFFF),
 * lw 0x34($a0) into sw 0x34($v0) and lw 0x38($a0) into sw 0x38($v0), each load
 * followed by its delay nop. The epilogue is lw $ra,0x10($sp); addiu
 * $sp,$sp,0x18; jr $ra; nop, so the span is one complete function with a
 * single return path and no jal caller in this payload.
 *
 * Name evidence: record byte +5 is the front-end entry table's handler
 * selector - the shared RecordSlot view in include/bof3/ui/game00_internal.h
 * documents 0x05 as the handler selector for the table at 0x801C7C70, and
 * src/bof3/ui/func_8019A0E4.c dispatches that byte through D_801C7C70 - so this
 * body writes handler selector 0x1C. Byte +0 is the occupancy flag the whole
 * family sets, and the +0x34/+0x38 pair is the scratch work object's position
 * pair: the family's exact siblings
 * claimFreeWorkRecordSeedWorkPositionWorld03Area13413_801F48B0 and
 * claimFreeWorkRecordSeedPositionAndFadeWorld04Area18813_801F3DE0 read that
 * same pair from the 0x1F800044 cell, and
 * seedFrontEndRecordHandler37FromWorkPositionWorld01Area04313_801F2E28 is the
 * nearest sibling of this statement order (claim one record, set the occupancy
 * byte, write a +5 handler selector, then copy work-object position words).
 * The only in-image reference to 0x801F2CDC is the word 0x801F2CDC at payload
 * offset 0x9E4 (0x801F35E4), the fifth word of the five-word code-pointer run
 * at 0x801F35D4 that names this overlay's five code bodies in ascending order;
 * there is no jal, so the body is reached from outside its own payload. What
 * that run dispatches and how it is indexed is not proven here: the words after
 * it address payload offsets that disassemble as data, so the run is not a
 * whole-table description.
 *
 * Shape evidence: the statement order, the single row-base pointer and the
 * pointer local for the work object come from those exact siblings; this
 * function's own selector byte 0x1C, its two-word copy and its cell binding come
 * from the original bytes. The value 0x1C has no consumer inside this payload,
 * so it stays described by value rather than named.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedFrontEndRecordHandler1CFromWorkPositionWorld00Area01013_801F2CDC(
    void) {
  u8* record;
  u8* work;
  u8 index;

  index = func_8019601C();
  if (index != 0xFF) {
    record = &D_80143FC8[(u32)index * 0x74u];
    record[0] = 1;
    record[5] = 0x1C;
    work = D_1F800044;
    *(u32*)(record + 0x34) = *(u32*)(work + 0x34);
    *(u32*)(record + 0x38) = *(u32*)(work + 0x38);
  }
}

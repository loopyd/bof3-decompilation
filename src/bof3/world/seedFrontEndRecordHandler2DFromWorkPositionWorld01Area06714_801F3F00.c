#include "bof3/bof3.h"

/* Front-end record table at 0x80143FC8: the shared allocator hands out indices
 * into it and every row is 0x74 bytes (the original scales the index by 0x74).
 * This body proves the occupancy byte at +0, the handler selector at +5 and the
 * work-position word pair at +0x34/+0x38. */
extern u8 D_80143FC8[]; /* @source 0x80143FC8 @kind bss */

/* Scratch work object published through the pointer cell at 0x1F800044. The
 * original materialises the cell as `lui 0x1F80` plus `lw 0x44(reg)` and keeps
 * the loaded pointer for both position words, so the named cell symbol - not
 * the scratchpad accessor, which folds to the same pair - is the binding, and
 * one pointer local serves the pair. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Front-end record allocator: returns the index of the next free 0x74-byte row
 * at 0x80143FC8, or 0xFF when the table is full. */
u8 func_8019601C(void);

/* @source 0x801F3F00
 * @behavior Seeds one claimed front-end record from the current work position:
 * it claims the record through the shared allocator func_8019601C and, unless
 * the returned index is 0xFF, seeds the claimed 0x74-byte row at 0x80143FC8 -
 * byte +0 takes 1 (the occupancy flag the whole family sets) and byte +5 takes
 * the handler selector 0x2D - then copies the current work object's position
 * pair: the row's words at +0x34 and +0x38 take words 0x34 and 0x38 of the
 * scratch work object published at 0x1F800044. Takes no arguments and returns
 * nothing; a full table (index 0xFF) leaves every row untouched.
 *
 * Evidence: payload asm at 0x801F3F00 - addiu $sp,-0x18 and sw $ra,0x10($sp),
 * then jal func_8019601C, andi $v1,$v0,0xFF and the 0xFF compare in $v0 with
 * `sll $v0,$v1,3` in the branch delay slot; the index scale is the compiler's
 * 0x74 form `sll 3 / subu / sll 2 / addu / sll 2`, then lui/addiu of
 * D_80143FC8 into $v1 and `addu $v0,$v0,$v1`, so all four stores address one
 * computed row base ($v0): sb 1 at 0x0, sb 0x2D at 0x5, and the word pair -
 * lui $a0,(0x1F800044 >> 16) plus lw $a0,(0x1F800044 & 0xFFFF), lw 0x34($a0)
 * into sw 0x34($v0) and lw 0x38($a0) into sw 0x38($v0), each load followed by
 * its delay nop. The epilogue is lw $ra,0x10($sp); addiu $sp,$sp,0x18; jr $ra;
 * nop, so the span is one complete function with a single return path.
 *
 * Name evidence: record byte +5 is the front-end entry table's handler
 * selector - the family's existing exact lifts name that byte the same way
 * (claimFrontEndRecordAndSetHandlerBaWorld03Area12114_801F2DC0,
 * seedFrontEndRecordHandler1CFromWorkPositionWorld00Area01013_801F2CDC,
 * seedFrontEndRecordHandler25World01Area07113_801F2CF4) - so this body writes
 * handler selector 0x2D. Byte +0 is the occupancy flag the whole family sets.
 * The +0x34/+0x38 pair is the scratch work object's position pair, the same
 * two words the family's exact siblings copy from the 0x1F800044 cell.
 *
 * Shape evidence: seedFrontEndRecordHandler1CFromWorkPositionWorld00Area01013_801F2CDC
 * is instruction-for-instruction identical to this span except for the handler
 * constant (0x1C here 0x2D) and the relocated symbol addresses, so this body
 * reuses that proven statement order and the single row-base pointer. The value
 * 0x2D has no consumer inside this payload, so it stays described by value
 * rather than named.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedFrontEndRecordHandler2DFromWorkPositionWorld01Area06714_801F3F00(
    void) {
  u8* record;
  u8* work;
  u8 index;

  index = func_8019601C();
  if (index != 0xFF) {
    record = &D_80143FC8[(u32)index * 0x74u];
    record[0] = 1;
    record[5] = 0x2D;
    work = D_1F800044;
    *(u32*)(record + 0x34) = *(u32*)(work + 0x34);
    *(u32*)(record + 0x38) = *(u32*)(work + 0x38);
  }
}

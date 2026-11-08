#include "bof3/bof3.h"

/* Shared front-end record table at 0x80143FC8. The original scales the index
 * returned by the allocator by the 0x74-byte row stride and addresses one
 * computed row base; this body touches row byte 0, row byte 1, row byte 5 and
 * the position triple at +0x34/+0x38/+0x3C only. */
extern u8 D_80143FC8[]; /* @source 0x80143FC8 @kind bss */

/* Scratch work object published through the fixed pointer cell at 0x1F800044.
 * The original materialises the cell with `lui 0x1F80` plus `lw 0x44(reg)` and
 * keeps the loaded pointer in one register across all three position words, so
 * the cell symbol is the binding and one pointer local serves the triple. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Front-end record allocator: returns the index of the next free 0x74-byte
 * record at 0x80143FC8, or 0xFF when the table is full. */
u8 func_8019601C(void);

/* @source 0x801F557C
 * @behavior Seeds one claimed front-end record from the current work position:
 * it claims the record through the shared allocator func_8019601C and, unless
 * the returned index is 0xFF, fills the claimed 0x74-byte row at 0x80143FC8 -
 * byte +0 takes 1, the occupancy flag the family sets, byte +5 takes the
 * handler selector 0x90, and byte +1 is cleared - then copies the work object
 * position triple: the row words at +0x34, +0x38 and +0x3C take words 0x34,
 * 0x38 and 0x3C of the scratch work object published at 0x1F800044. Takes no
 * arguments and returns nothing; a full table (index 0xFF) leaves every row
 * untouched.
 *
 * Evidence from the original bytes at 0x801F557C: the span is one complete
 * 0x90-byte function - addiu $sp,-0x18 and sw $ra,0x10($sp) in front, one
 * jal func_8019601C, and the epilogue lw $ra,0x10($sp); addiu $sp,$sp,0x18;
 * jr $ra; nop - with a single return path. The allocator result is masked with
 * andi $v1,$v0,0xFF and compared against addiu $v0,$zero,0xFF inside the
 * branch, whose delay slot starts the index scale `sll $v0,$v1,3`; the scale
 * is the 0x74 form sll 3 / subu / sll 2 / addu / sll 2 on the allocator
 * result, then lui/addiu of D_80143FC8 into $v1 and `addu $v0,$v0,$v1`, so
 * every store addresses one computed row base ($v0): addiu $v1,$zero,1 into
 * sb 0x0($v0), addiu $v1,$zero,0x90 into sb 0x5($v0), sb $zero,0x1($v0), then
 * lui $a0,(0x1F800044 >> 16) plus lw $a0,(0x1F800044 & 0xFFFF) and the word
 * triple - lw 0x34($a0) into sw 0x34($v0), lw 0x38($a0) into sw 0x38($v0) and
 * lw 0x3C($a0) into sw 0x3C($v0), each load followed by its delay nop. The
 * cell is loaded once and never re-read.
 *
 * Body evidence: this span is byte-identical to the already-exact sibling
 * seedFrontEndRecordHandler90AndClearRowByte1FromWorkPositionWorld03Area13413_801F54EC
 * - one cmp of the two 0x90-byte payload spans at their own
 * runtime-minus-load offsets reports equality, including the selector
 * immediate 0x90 and the row byte +1 clear - so the statement order, the
 * single row base pointer, the byte-local allocator result and the pointer
 * local for the work object are copied from that body with this function own
 * address tag. The same-target representative makes the clone adapter
 * ineligible (it refuses a representative from the clone target own binary),
 * so the copy is written by hand.
 *
 * Reachability: no jal in this payload targets 0x801F557C. Its only in-image
 * reference is the word 0x801F557C at payload offset 0x3768 (0x801F6368), the
 * fifteenth entry - index 14 - of the eighteen-pointer code run based at
 * 0x801F6330, one slot after the identical 0x801F54EC twin at index 13, so
 * the overlay reaches this body indirectly through that run. The slot index
 * is not encoded in the body, so the role stays the one the bytes prove.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedFrontEndRecordHandler90AndClearRowByte1FromWorkPositionWorld03Area13413_801F557C(
    void) {
  u8* record;
  u8* work;
  u8 index;

  index = func_8019601C();
  if (index != 0xFF) {
    record = &D_80143FC8[(u32)index * 0x74u];
    record[0] = 1;
    record[5] = 0x90;
    record[1] = 0;
    work = D_1F800044;
    *(u32*)(record + 0x34) = *(u32*)(work + 0x34);
    *(u32*)(record + 0x38) = *(u32*)(work + 0x38);
    *(u32*)(record + 0x3C) = *(u32*)(work + 0x3C);
  }
}

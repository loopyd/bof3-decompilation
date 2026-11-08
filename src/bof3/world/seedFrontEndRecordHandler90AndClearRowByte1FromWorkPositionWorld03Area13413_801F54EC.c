#include "bof3/bof3.h"

/* Shared front-end record table at 0x80143FC8: the shared allocator hands out
 * indices into it and the original scales the index by the 0x74-byte row
 * stride. This body touches the row byte 0, the row byte 1, the row byte 5 and
 * the position triple at +0x34/+0x38/+0x3C; the rest of the row is not touched
 * here. */
extern u8 D_80143FC8[]; /* @source 0x80143FC8 @kind bss */

/* Scratch work object published through the pointer cell at 0x1F800044. The
 * original materialises the fixed cell with `lui 0x1F80` plus `lw 0x44(reg)`
 * and keeps the loaded pointer in one register across all three position
 * words, so the named cell symbol - not a scratchpad accessor - is the
 * binding and one pointer local serves the triple. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Front-end record allocator: returns the index of the next free 0x74-byte
 * record at 0x80143FC8, or 0xFF when the table is full. */
u8 func_8019601C(void);

/* @source 0x801F54EC
 * @behavior Seeds one claimed front-end record from the current work position:
 * it claims the record through the shared allocator func_8019601C and, unless
 * the returned index is 0xFF, seeds the claimed 0x74-byte row at 0x80143FC8 -
 * byte +0 takes 1, the occupancy flag the whole family sets, byte +5 takes the
 * handler selector 0x90, and byte +1 is cleared to zero - then copies the
 * current work object's position triple: the row's words at +0x34, +0x38 and
 * +0x3C take words 0x34, 0x38 and 0x3C of the scratch work object published at
 * 0x1F800044. Takes no arguments and returns nothing; a full table (index
 * 0xFF) leaves every row untouched.
 *
 * Evidence from the original bytes at 0x801F54EC: the span is one complete
 * 0x90-byte function - addiu $sp,-0x18 and sw $ra,0x10($sp) in front, a single
 * jal func_8019601C, and the epilogue lw $ra,0x10($sp); addiu $sp,$sp,0x18;
 * jr $ra; nop - with a single return path. The allocator result is masked with
 * andi $v1,$v0,0xFF and compared against addiu $v0,$zero,0xFF inside the
 * branch, whose delay slot already starts the index scale `sll $v0,$v1,3`; the
 * scale is the compiler's 0x74 form sll 3 / subu / sll 2 / addu / sll 2 on the
 * allocator result, then lui/addiu of D_80143FC8 into $v1 and
 * `addu $v0,$v0,$v1`, so every store addresses one computed row base ($v0):
 * addiu $v1,$zero,1 into sb 0x0($v0), addiu $v1,$zero,0x90 into sb 0x5($v0),
 * sb $zero,0x1($v0), then lui $a0,(0x1F800044 >> 16) plus
 * lw $a0,(0x1F800044 & 0xFFFF) and the word triple - lw 0x34($a0) into
 * sw 0x34($v0), lw 0x38($a0) into sw 0x38($v0) and lw 0x3C($a0) into
 * sw 0x3C($v0), each load followed by its delay nop. The cell is loaded once
 * and never re-read.
 *
 * Name evidence: record byte +5 is the front-end entry table's handler
 * selector - the shared RecordSlot view in include/bof3/ui/game00_internal.h
 * documents 0x05 as the handler selector of the table at 0x801C7C70, and
 * src/bof3/ui/func_8019A0E4.c dispatches that byte - and this body writes
 * selector 0x90, so the role is proven and only the selector's own consumer,
 * which is outside this payload, stays described by value. Byte +0 is the
 * occupancy flag the family sets, and the +0x34/+0x38/+0x3C words are the
 * scratch work object's position triple. Byte +1 has no proven role - the
 * shared view still spells it as an unknown byte - so the clear this body
 * writes there stays anchored to the row offset in the name. The statement
 * order, the single row base pointer, the byte-local allocator result and the
 * pointer local for the work object come from this payload's exact sibling
 * seedFrontEndRecordHandler93FromWorkPositionWorld03Area13413_801F560C, which
 * runs the same claim/set/copy order over the same table and stride; it
 * differs only in its selector immediate and by not clearing row byte +1, so
 * both are written here from this function's own bytes. The cross-target
 * family member seedFrontEndRecordHandler1CFromWorkPositionWorld00Area01013_801F2CDC
 * carries the shorter word pair and is shape evidence only.
 *
 * Reachability: no jal in this payload targets 0x801F54EC. Its only in-image
 * reference is the word 0x801F54EC at payload offset 0x3764 (0x801F6364), the
 * fourteenth entry of the eighteen-pointer code run at 0x801F6330 inside the
 * 0x801F5724 tail blob, so the overlay reaches this body indirectly through
 * that run.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedFrontEndRecordHandler90AndClearRowByte1FromWorkPositionWorld03Area13413_801F54EC(
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

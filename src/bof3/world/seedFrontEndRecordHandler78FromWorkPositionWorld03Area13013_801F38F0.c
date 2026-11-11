#include "bof3/bof3.h"

/* Shared front-end entry table at 0x80143FC8: 20 records of 0x74 bytes, the
 * layout include/bof3/ui/game00_internal.h records as RecordSlot. The original
 * scales the allocator's index by the 0x74-byte row stride and addresses one
 * computed row base; this body touches row byte +0, row byte +5 and the word
 * triple at +0x34/+0x38/+0x3C only.
 * @source 0x80143FC8 @kind bss
 */
extern u8 D_80143FC8[];

/* Scratchpad work-pointer cell at 0x1F800044, the cursor spelling this target
 * map and its lifters already use. The original materialises the cell with
 * `lui 0x1F80` plus `lw 0x44(reg)` and keeps the loaded pointer in one register
 * across all three position words.
 * @source 0x1F800044 @kind data
 */
extern u8* workRecordCursor1F800044;

/* Shared front-end record allocator at 0x8019601C: returns the index of the
 * next free 0x74-byte record at 0x80143FC8, or 0xFF when the table is full.
 * Outside this payload and called only from it, so it keeps the generated raw
 * name.
 */
u8 func_8019601C(void);

/* @source 0x801F38F0
 * @behavior Seeds one front-end record from the current work position: it
 *           claims the record through the shared allocator func_8019601C and,
 *           unless the returned index is 0xFF, fills the claimed 0x74-byte row
 *           at 0x80143FC8 - byte +0 takes 1, the occupancy flag the family
 *           sets, and byte +5 takes the handler selector 0x78 - then copies the
 *           work object's position triple: the row words at +0x34, +0x38 and
 *           +0x3C take words 0x34, 0x38 and 0x3C of the scratch work object
 *           published at 0x1F800044. Takes no arguments and returns nothing; a
 *           full table (index 0xFF) leaves every row untouched.
 *
 * Shape evidence from the original bytes at 0x801F38F0: the span is one
 * complete 0x8C-byte function - addiu $sp,-0x18 and sw $ra,0x10($sp) in front,
 * one jal func_8019601C, and the epilogue lw $ra,0x10($sp); addiu $sp,$sp,0x18;
 * jr $ra; nop - with a single return path. The allocator result is masked with
 * andi $v1,$v0,0xFF and compared against addiu $v0,$zero,0xFF inside the
 * branch, whose delay slot starts the index scale `sll $v0,$v1,3`; the scale is
 * the 0x74 form sll 3 / subu / sll 2 / addu / sll 2 on the allocator result,
 * then lui/addiu of D_80143FC8 into $v1 and `addu $v0,$v0,$v1`, so every store
 * addresses one computed row base ($v0): addiu $v1,$zero,1 into sb 0x0($v0),
 * addiu $v1,$zero,0x78 into sb 0x5($v0), then lui $a0,(0x1F800044 >> 16) plus
 * lw $a0,(0x1F800044 & 0xFFFF) and the word triple - lw 0x34($a0) into
 * sw 0x34($v0), lw 0x38($a0) into sw 0x38($v0) and lw 0x3C($a0) into
 * sw 0x3C($v0), each load followed by its delay nop. The cell is loaded once
 * and never re-read, and this body clears no further row byte.
 *
 * Seed evidence: the 0x8C-byte body is word-identical to the already-exact
 * sibling seedFrontEndRecordHandler93FromWorkPositionWorld03Area13413_801F560C
 * at 0x801F560C in the area134 payload - one cmp of the two 0x8C-byte payload
 * spans at their own runtime-minus-load offsets reports a single differing
 * word, the selector immediate addiu $v1,$zero,0x93 there against 0x78 here -
 * so the
 * statement order, the single row base pointer, the byte-local allocator result
 * and the pointer local for the work object are taken from that body with this
 * function's own selector literal and own address tag. The 0x8C-byte pattern
 * occurs exactly once across the shipped payloads, at this address, so the
 * clone lever has no byte-identical source and is declined. The nearer
 * same-payload shapes at 0x801F54EC/0x801F557C in area134 also clear row byte
 * +1 and write selector 0x90, so they are shape evidence only.
 *
 * Name evidence: record byte +5 is the front-end entry table's handler
 * selector - the RecordSlot view in include/bof3/ui/game00_internal.h documents
 * 0x05 as the handler selector of the table at 0x80143FC8, and this body
 * writes selector 0x78 there - while the +0x34/+0x38/+0x3C words are the
 * scratch work object's position triple, so the role is proven and only the
 * selector's own consumer, which is outside this payload, stays described by
 * value.
 *
 * Reachability: no jal in this payload targets 0x801F38F0. Its only in-image
 * reference is the word 0x801F38F0 at payload offset 0x23F0 (0x801F4FF0), the
 * ninth entry - index 8 - of the pointer run that starts at 0x801F4FD0 inside
 * the 0x801F3ED8 tail blob, one slot after the identical 0x801F3870 seeder, so
 * the overlay reaches this body indirectly through that run. The slot index is
 * not encoded in the body, so the role stays the one the bytes prove.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedFrontEndRecordHandler78FromWorkPositionWorld03Area13013_801F38F0(
    void) {
  u8* record;
  u8* work;
  u8 index;

  index = func_8019601C();
  if (index != 0xFF) {
    record = &D_80143FC8[(u32)index * 0x74u];
    record[0] = 1;
    record[5] = 0x78;
    work = workRecordCursor1F800044;
    *(u32*)(record + 0x34) = *(u32*)(work + 0x34);
    *(u32*)(record + 0x38) = *(u32*)(work + 0x38);
    *(u32*)(record + 0x3C) = *(u32*)(work + 0x3C);
  }
}

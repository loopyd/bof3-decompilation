#include "bof3/bof3.h"

/* Work record published through the scratchpad pointer cell at 0x1F800044. The
 * original re-materialises the cell once per access group - the prologue, the
 * halfword read, the two byte-8 reads and the two stores each open their own
 * `lui 0x1F80` plus folded `lw 68` pair - so the cell stays a named pointer
 * symbol that every access re-reads.
 * @source 0x1F800044 @kind data */
extern u8* D_1F800044;

/* Local work record published through the global pointer cell at 0x80146250;
 * this body only proves the byte at +0x118.
 * @source 0x80146250 @kind unknown */
extern u8* D_80146250;

/* Front-end record table at 0x80143FC8 - 0x74 bytes per record (the original
 * scales the allocator index by 0x74). Only the two bytes this body writes are
 * declared, each at its own byte base because the original re-materialises
 * `lui $at,0x8014` for both row stores.
 * @source 0x80143FC8 @kind bss */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];

/* Shared EXE-side cue dispatcher. Reviewed call sites pass both constants and
 * computed ids; this body hands over an int-typed sum and the original emits no
 * truncation (`lhu` then `addiu $a0,$a0,0x100`), so the width here is the wider
 * u32 form rather than the u16 one. */
void func_8015DF18(u32 arg0);

/* Shared EXE-side work-sequence selector; the original truncates the sequence
 * id to a byte with `andi $a0,$a0,0xFF`. */
u32 func_8014D8D4(u8 arg0);

/* Front-end record allocator: returns the index of the next free 0x74-byte
 * record at 0x80143FC8, or 0xFF when the table is full. */
u8 func_8019601C(void);

/* @source 0x801F2C40
 * @behavior Per-frame step of the AREA099 slot-13 work record published at the
 * scratchpad pointer cell 0x1F800044. It first rewinds the record byte at
 * +0x08 by one modulo 8, but only while bit 0 of that byte is clear, so an odd
 * byte holds. It then queues the shared sound cue formed from the record's
 * unsigned halfword at +0x2C plus 0x100, requests the shared work sequence
 * ((byte 0x08 - 1) / 2 + 0x42) truncated to a byte, and sets bit 0x40 of byte
 * +0x118 of the local work record published through the global pointer cell
 * 0x80146250. Finally it claims one front-end record through the shared
 * allocator func_8019601C, publishes the returned index in the record byte at
 * +0x0B and, unless that byte reads 0xFF (the allocator's table-full result),
 * seeds the claimed 0x74-byte row at 0x80143FC8 with the occupancy byte +0 = 1
 * and the handler selector 0x1B at +0x05. The row's two stores re-read the
 * index through the pointer cell, so the first store shares the test's load and
 * the second opens a fresh cell load. Takes no arguments and returns nothing,
 * and the 0x18-byte frame keeps only $ra at 0x10 across the three calls.
 *
 * Evidence: the payload span 0x801F2C40-0x801F2D68 is one complete function -
 * the scheduled prologue `lui/lw` of the 0x1F800044 cell with `addiu $sp,-0x18`
 * and `sw $ra,0x10($sp)` behind it, three `jal`s and the single epilogue
 * `lw $ra,0x10($sp); addiu $sp,$sp,0x18; jr $ra; nop`. The only in-image
 * reference to it is the word 0x801F2C40 at 0x801F4934, the third entry of the
 * code-pointer run 0x801F492C-0x801F4964, so it is reached from outside its own
 * payload and has no `jal` caller. Its own literals - the cue bias 0x100 from
 * the +0x2C halfword, the sequence bias 0x42, the 0x40 flag bit and the row
 * values 1 and 0x1B - are read from these original bytes.
 *
 * Name evidence: byte +0x05 of a 0x74-byte row at 0x80143FC8 is the front-end
 * entry table's handler selector (the shared RecordSlot view documented for the
 * table at 0x801C7C70, dispatched through D_801C7C70 in src/bof3/ui), and byte
 * +0 is the occupancy flag the family sets, so this body claims the row with
 * handler selector 0x1B. The record byte at +0x0B is where this family
 * publishes the claimed index.
 *
 * Shape evidence: the statement order, the named-cell access spelling and the
 * `(u8)((byte 8 - 1) / 2 + 0x42)` form come from the exact siblings
 * refreshLayersThenSelectWorkSequenceFromByte8HalfArmCountdownAndAdvanceWorkByte3PlcharPlp01600_801D0060
 * and func_801F92DC (the 0x74-scale row stores), with this function's own
 * gate, stored literals and call order from the original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkByte8QueueWorkCueSelectSequenceAndClaimFrontEndRecordHandler1BWorld02Area09913_801F2C40(
    void) {
  if ((D_1F800044[0x8] & 1u) == 0u) {
    D_1F800044[0x8] = (u8)((D_1F800044[0x8] - 1) & 7u);
  }

  func_8015DF18(*(u16*)(D_1F800044 + 0x2c) + 0x100);
  func_8014D8D4((u8)((D_1F800044[0x8] - 1) / 2 + 0x42));

  D_80146250[0x118] |= 0x40u;
  D_1F800044[0xb] = func_8019601C();

  if (D_1F800044[0xb] != 0xffu) {
    D_80143FC8[(u32)D_1F800044[0xb] * 0x74u] = 1u;
    D_80143FCD[(u32)D_1F800044[0xb] * 0x74u] = 0x1bu;
  }
}

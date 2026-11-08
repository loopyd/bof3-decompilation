#include "bof3/bof3.h"

/* Work record published through the scratchpad pointer cell at 0x1F800044. The
 * original re-materialises the cell for every access group - the byte-8 test,
 * the allocator store, the claim test, both row stores, the sequence selector,
 * the cue dispatcher and the step byte each open their own `lui 0x1F80` plus
 * folded `lw 68` pair - so the cell stays a named pointer symbol that every
 * access re-reads.
 * @source 0x1F800044 @kind data */
extern u8* D_1F800044;

/* Front-end record table at 0x80143FC8 - 0x74 bytes per record, the scale this
 * body uses. Only the two bytes it writes are declared, each at its own byte
 * base because the original re-materialises `lui $at,0x8014` for both row
 * stores.
 * @source 0x80143FC8 @kind bss */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];

/* Shared EXE-side cue dispatcher. The original widens the record's unsigned
 * halfword at +0x2C with `lhu` and adds 0x100 with `addiu`, so the call-site
 * width is the wider u32 form rather than the u16 one. */
void func_8015DF18(u32 arg0);

/* Shared EXE-side work-sequence selector; the original narrows the computed
 * sequence id with `srl`, leaving no `andi` before the call. */
u32 func_8014D8D4(u8 arg0);

/* Front-end record allocator: returns the index of the next free 0x74-byte
 * record at 0x80143FC8, or 0xFF when the table is full. */
u8 func_8019601C(void);

/* @source 0x801CF4CC
 * @behavior Per-frame step of the work record published at the scratchpad
 *           pointer cell 0x1F800044. It first rewinds that record's byte +0x08
 *           by one modulo 8, but only while bit 0 of the byte is clear, so an
 *           odd byte holds. It then claims one front-end record through the
 *           shared allocator func_8019601C, publishes the returned index in the
 *           record's byte +0x0B and, unless that byte reads 0xFF (the
 *           allocator's table-full result), seeds the claimed 0x74-byte row at
 *           0x80143FC8 with the occupancy byte +0 = 1 and the handler selector
 *           0x1B at +0x05. The row's two stores re-read the index through the
 *           pointer cell: the first shares the test's load, the second opens a
 *           fresh cell load. It then selects the shared work sequence
 *           (byte 0x08 >> 1) + 0x42 and queues the shared sound cue formed from
 *           the record's unsigned halfword at +0x2C plus 0x100, in that order,
 *           and finally advances the record's dispatch byte at +0x02 by one.
 *           Takes no arguments and returns nothing, and the 0x18-byte frame
 *           keeps only $ra at 0x10 across the three calls.
 *
 * Evidence: the payload span 0x801CF4CC-0x801CF5E4 is one complete function -
 * the scheduled prologue `lui/lw` of the 0x1F800044 cell with
 * `addiu $sp,-0x18` and `sw $ra,0x10($sp)` behind it, three `jal`s and the
 * single epilogue `lw $ra,0x10($sp); sb` (the step store) `; addiu $sp,$sp,0x18;
 * jr $ra; nop`. Its own literals - the modulo-8 rewind with the odd-byte hold,
 * the 0xFF table-full test, the row values 1 and 0x1B, the sequence bias 0x42
 * applied to the halved byte, the cue bias 0x100 on the +0x2C halfword and the
 * +0x02 step increment - are read from these original bytes.
 *
 * Name evidence: byte +0x05 of a 0x74-byte row at 0x80143FC8 is the front-end
 * entry table's handler selector (the family sibling
 * claimFrontEndRecordHandler34FromPositionPairStoreStateDistanceAndClearRowByteB
 * writes the same pair of row bytes, and the shared RecordSlot view documents
 * the selector byte), and byte +0 is the occupancy flag the family sets, so
 * this body claims the row with handler selector 0x1B. The record byte at
 * +0x0B is where this family publishes the claimed index, byte +0x08 is the
 * even/odd step byte the family rewinds modulo 8, and byte +0x02 is the
 * dispatch byte the family advances.
 *
 * Shape evidence: the statement order, the named-cell access spelling and the
 * `(u8)((byte 8 >> 1) + 0x42)` form come from the exact siblings
 * stepWorkByte8QueueWorkCueSelectSequenceAndClaimFrontEndRecordHandler1BWorld02Area09913_801F2C40
 * (same eight accesses, a different order and a different last store) and
 * refreshLayersThenSelectWorkSequenceFromByte8HalfArmCountdownAndAdvanceWorkByte3PlcharPlp01600_801D0060,
 * with this function's own order, literals and single-store last step from the
 * original bytes. The 280-byte body is not byte-identical to any lifted
 * sibling, so it is a seed and not a clone claim.
 * @status exact
 * @match 100.00
 * @residual none
 */
void adjustEvenWorkByte8ClaimFrontEndRecordHandler1BSelectSequenceAndQueueCueThenAdvanceWorkByte2PlcharPlp47800_801CF4CC(
    void) {
  if ((D_1F800044[0x8] & 1u) == 0u) {
    D_1F800044[0x8] = (u8)((D_1F800044[0x8] - 1) & 7u);
  }

  D_1F800044[0xb] = func_8019601C();

  if (D_1F800044[0xb] != 0xffu) {
    D_80143FC8[(u32)D_1F800044[0xb] * 0x74u] = 1u;
    D_80143FCD[(u32)D_1F800044[0xb] * 0x74u] = 0x1bu;
  }

  func_8014D8D4((u8)((D_1F800044[0x8] >> 1) + 0x42));
  func_8015DF18(*(u16*)(D_1F800044 + 0x2c) + 0x100);

  D_1F800044[0x2]++;
}

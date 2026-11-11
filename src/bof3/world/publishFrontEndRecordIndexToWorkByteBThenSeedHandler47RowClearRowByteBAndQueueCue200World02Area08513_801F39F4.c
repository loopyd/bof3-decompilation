#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * handlers. The original re-materialises the cell address with its own lui
 * before every load, including the loads that follow the byte +0x0B store with
 * no call in between, and the store through the loaded record forces each later
 * access to reload the cell, so each access names the cell instead of keeping a
 * pointer in a local. The exact sibling
 * applyMainLookupRecord184ClearWorkByte24Bit0RequestResource0ThenSetWorkByte2AAndByte0Flag10World02Area08513_801F3AF8
 * of this target uses the same spelling.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;
/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8. This body addresses every field through its own
 * absolute symbol, because the original re-materialises the base and adds the
 * scaled row offset for each access, so each field base stays separate.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* Row byte 5, the entry handler selector; this body seeds it with 0x47.
 * @source 0x80143FCD @kind unknown
 */
extern u8 D_80143FCD[];
/* Row byte 0x0B. The other claim bodies of this family write 1 there, while
 * this body clears it, so its role is not established.
 * @source 0x80143FD3 @kind unknown
 */
extern u8 D_80143FD3[];
/* Record published through the global pointer cell at 0x80148210; this body
 * reads and writes back the halfword at its +0x0A offset through one base.
 * @source 0x80148210 @kind unknown
 */
extern u16* D_80148210;

/* Front-end record allocator of the shared main executable, outside this
 * payload: it answers the index of the next free 0x74-byte row at 0x80143FC8,
 * or 0xFF when every row is occupied. This target's generated
 * undefined_funcs_auto.txt binds the address-anchored name. */
extern u8 func_8019601C(void); /* @source 0x8019601C */

/* Cue selector of the shared main executable, outside this payload; the
 * sibling lifts of the front-end family pass it cue ids. This target's
 * generated undefined_funcs_auto.txt binds the address-anchored name. */
extern void func_8015DF18(u32 cue); /* @source 0x8015DF18 */

/* @source 0x801F39F4
 * @behavior Overlay step: it asks the shared free-record selector
 *           func_8019601C for a row index and, without masking the answer,
 *           stores it into work byte +0x0B of the record behind the scratchpad
 *           pointer cell 0x1F800044. It then re-reads that byte through a
 *           fresh cell load and, unless the byte reads 0xFF (the selector's
 *           table-full result), seeds the row it selects in the 0x74-byte
 *           front-end record table at 0x80143FC8: byte +0x00 takes 1, the
 *           occupancy flag this whole family sets, byte +0x05 takes handler
 *           selector 0x47, byte +0x0B takes 0, and the body then queues cue
 *           0x200 through func_8015DF18. On the table-full arm it instead
 *           subtracts 2 from the halfword at +0x0A of the record published
 *           through the global pointer cell 0x80148210. It takes no arguments,
 *           returns nothing, and its 0x18-byte frame holds only $ra at 0x10
 *           across the two calls; the 0x200 argument materialises in the second
 *           call's delay slot and the first call keeps a `nop` there. The
 *           taken arm ends on a `j` to the shared epilogue.
 *
 * Evidence: the payload span at offset 0xDF4 of the overlay image is 0x104
 * bytes with the prologue `addiu $sp,-0x18` / `sw $ra,0x10($sp)`, `jal
 * func_8019601C`, then `lui $v1,0x1F80` / `lw $v1,0x44($v1)` /
 * `sb $v0,0xB($v1)`; the test reloads the cell into $v0 and does
 * `lbu $v1,0xB($v0)` / `addiu $v0,$zero,0xFF` / `beq` with `sll $v0,$v1,3` in
 * the branch delay slot, and each record store walks the compiler's 0x74 form
 * `sll 3 / subu / sll 2 / addu / sll 2` on a re-read `lbu 0xB` before
 * `lui $at,%hi(SYM)` / `addu $at,$at,$v0` / store `%lo(SYM)($at)`. The
 * table-full arm is `lui $v1,%hi(D_80148210)` / `lw $v1,%lo(...)` /
 * `lhu $v0,0xA($v1)` / `addiu $v0,$v0,-2` / `sh`, and the span closes with
 * `lw $ra,0x10($sp)` / `addiu $sp,$sp,0x18` / `jr $ra` / `nop`, so it is one
 * complete function with a single return.
 *
 * Name and shape evidence: the nearest sibling is this target's own
 * 0x801F38F0, which is the same 0x104-byte body and differs only in the last
 * row store, which writes 1 there and 0 here, and in the register that carries
 * that constant; it sits on a `c`-less `asm` row, so its disassembly is the
 * shape proof rather than a claim, and the statement order, operand forms and
 * absolute-field spelling here are its order. Row byte +0x05 is the front-end
 * entry handler selector and row byte +0x00 the occupancy flag, as the exact
 * lifts claimFrontEndRecordSetHandler85AndStoreWorkRecordIndexElseLowerHalfword0aBy2World03Area14413_801F3BAC
 * and
 * publishFrontEndRecordIndexToWorkByteBThenSeedHandlerA7RecordFromWorkPositionWorld04Area19813_801F3854
 * of the same table record; the second of those is also this body's first four
 * statements plus its table-full arm, and it is byte-identical to them through
 * 0x801F4D98. A scan of every payload under out/binaries for the six-word
 * 0x47 row-store chain finds only this target's two spans (0xD60 and 0xE64), so
 * no other target holds this body and clone is not a lever here. The value
 * 0x47 has no consumer inside this payload, so it stays described by value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishFrontEndRecordIndexToWorkByteBThenSeedHandler47RowClearRowByteBAndQueueCue200World02Area08513_801F39F4(
    void) {
  D_1F800044[0x0B] = func_8019601C();

  if (D_1F800044[0x0B] != 0xFFu) {
    D_80143FC8[(u32)D_1F800044[0x0B] * 0x74u] = 1u;
    D_80143FCD[(u32)D_1F800044[0x0B] * 0x74u] = 0x47u;
    D_80143FD3[(u32)D_1F800044[0x0B] * 0x74u] = 0;
    func_8015DF18(0x200);
  } else {
    D_80148210[5] -= 2;
  }
}

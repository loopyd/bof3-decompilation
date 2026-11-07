#include "bof3/bof3.h"

/* Front-end record table and the field bases of its 0x74-byte rows: the
 * original re-materialises `lui at,0x8014` for every store, so each field is
 * addressed as its own absolute symbol rather than through one struct base.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];
/* @source 0x80143FFC @kind unknown */
extern u8 D_80143FFC[];
/* @source 0x80144000 @kind unknown */
extern u8 D_80144000[];
/* @source 0x80144004 @kind unknown */
extern u8 D_80144004[];
/* Scratchpad work-object pointer cell; the original re-materialises its
 * address for every access, so it keeps the symbol form.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;
/* Record published through the global pointer cell at 0x80148210; this body
 * reads and writes back the halfword at its +0x0A offset through one base.
 * @source 0x80148210 @kind unknown
 */
extern u16* D_80148210;

/* Front-end record allocator: returns the index of the next free 0x74-byte
 * record at 0x80143FC8, or 0xFF when the table is full. */
u8 func_8019601C(void);

/* @source 0x801F3854
 * @behavior Publishes the front-end record index returned by the shared
 *           allocator 0x8019601C into work byte +0x0B of the record behind the
 *           scratchpad pointer cell 0x1F800044, then re-reads that byte through
 *           a fresh cell load and seeds the record it selects at 0x80143FC8
 *           unless the byte came back 0xFF (the allocator's table-full result):
 *           byte +0x00 takes 1, the occupancy flag the whole front-end family
 *           sets, and byte +0x05 takes the handler selector 0xA7. The seeded
 *           row's words at +0x34 and +0x38 take the work object's words 0x34
 *           and 0x38, and +0x3C takes the work word 0x3C with 0x1000000 added,
 *           the raw-address spelling of a position word plus its bank tag. On
 *           the table-full arm it instead subtracts 2 from the halfword at
 *           +0x0A of the record published through the global pointer cell
 *           0x80148210. The single call keeps its `nop` delay slot, the
 *           0x18-byte frame holds only $ra at 0x10, and the taken arm ends on a
 *           `j` to the shared epilogue. Takes no arguments and returns nothing.
 *
 * Evidence: payload asm at 0x801F3854 - addiu $sp,-0x18 and sw $ra,0x10($sp),
 * jal 0x8019601C, then `lui $v1,0x1F80` plus `lw $v1,0x44($v1)` before
 * `sb $v0,0xB($v1)`; the compare re-reads the cell into $v0 and does
 * `lbu $v1,0xB($v0)`/`addiu $v0,$zero,0xFF`/`beq` with `sll $v0,$v1,3` in the
 * branch delay slot. Every record store walks the compiler's 0x74 form
 * `sll 3 / subu / sll 2 / addu / sll 2` on a re-read `lbu 0xB` and then spells
 * `lui $at,%hi(SYM)` / `addu $at,$at,$v0` / store `%lo(SYM)($at)` against the
 * absolute field symbols, so the occupancy and selector stores re-materialise
 * the cell and one shared `lw 0x44` base in $a0 serves the three word stores
 * (its `lbu 0xB` re-read at 0x801F3920 and 0x801F394C). The 0x3C store is
 * `lw $v1,0x3C($a0)` / `lui $a0,0x100` / `addu $v1,$v1,$a0`. The epilogue is
 * lw $ra,0x10($sp); addiu $sp,$sp,0x18; jr $ra; nop, so the span is one
 * complete function with a single return and no jal caller.
 *
 * Name evidence: byte +0x05 is the front-end entry table's handler selector
 * (the shared RecordSlot view in include/bof3/ui/game00_internal.h documents
 * 0x05 as that selector), and the exact siblings
 * publishFrontEndRecordIndexToWorkByteBAndDecrementRecordHalfword0aBy2WhenTableFullWorld02Area10814_801F4D60
 * (this body's first four statements plus the table-full arm, byte-identical
 * through 0x801F4D98) and
 * seedFrontEndRecordHandler1CFromWorkPositionWorld00Area01013_801F2CDC
 * (claim-then-seed with a +5 handler selector and the work position words)
 * supply the statement order and the family vocabulary; the value 0xA7 has no
 * consumer inside this payload, so it stays described by value. The only
 * in-image reference to 0x801F3854 is the word 0x801F3854 at 0x801F4290,
 * entry 224 of the pointer run the Splat row D_801F3F10 places, so the body is
 * reached from outside its own code spans rather than by jal.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishFrontEndRecordIndexToWorkByteBThenSeedHandlerA7RecordFromWorkPositionWorld04Area19813_801F3854(void) {
  u8* work;

  D_1F800044[0x0B] = func_8019601C();

  if (D_1F800044[0x0B] != 0xFFu) {
    D_80143FC8[(u32)D_1F800044[0x0B] * 0x74u] = 1u;
    D_80143FCD[(u32)D_1F800044[0x0B] * 0x74u] = 0xA7u;

    work = D_1F800044;
    *(u32*)(D_80143FFC + (u32)work[0x0B] * 0x74u) = *(u32*)(work + 0x34);
    *(u32*)(D_80144000 + (u32)work[0x0B] * 0x74u) = *(u32*)(work + 0x38);
    *(u32*)(D_80144004 + (u32)work[0x0B] * 0x74u) =
        *(u32*)(work + 0x3C) + 0x1000000u;
  } else {
    D_80148210[5] -= 2;
  }
}

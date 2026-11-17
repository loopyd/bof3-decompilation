#include "bof3/bof3.h"

/* Pointer cell of the scratchpad work record. This body loads the cell once for
 * its opening store group and re-reads it for every later access, so the cell
 * keeps the named form and the opening group reaches the record through one
 * local.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* The position-word triple of the 0x98-byte record table based at 0x80146888 of
 * the shared main executable, bound at the word 0x801468BC the scaled index
 * adds to: this body copies row words +0x00/+0x04/+0x08 of the base into the
 * work record's +0x34/+0x38/+0x3C. The neighbouring overlap dispatcher
 * 0x801F3B98 walks the same table as `0x80146888 + index * 0x98`, so the table
 * is the overlay's per-record template set and the index is work byte +0x07,
 * the record selector. Only the three copied words are proven, so the rest of
 * the row stays unnamed.
 * @source 0x801468BC @kind table
 */
typedef struct Area198RecordPosition {
  u32 word_34;
  u32 word_38;
  u32 word_3C;
  u8  unk_40[0x8C];
} Area198RecordPosition;
ASSERT_SIZE(Area198RecordPosition, 0x98);
extern Area198RecordPosition D_801468BC[];

/* Per-record step-entry table of this overlay's own image, 8 bytes per entry,
 * selected by work byte +0x06: an unsigned halfword at +0x00 that this body
 * hands to the shared main-lookup helper 0x8014DD3C and an unsigned byte at
 * +0x02 that it hands to the shared resource request 0x8014D6B8. The bytes at
 * 0x801F43D8 are 0x030C/0x01 for entry 0 and 0x030D/0x00 for entry 1; the five
 * bytes after +0x02 are not read by this body. Bound at the entry base; the
 * review row the Splat file places is the pointer run D_801F3F10 that this
 * interior address sits inside.
 * @source 0x801F43D8 @kind table
 */
typedef struct Area198StepEntry {
  u16 entry_00;
  u8  resource_02;
  u8  unk_03[5];
} Area198StepEntry;
ASSERT_SIZE(Area198StepEntry, 8);
extern Area198StepEntry D_801F43D8[];

/* Shared main-executable lookup helper: it applies the "main lookup" record
 * whose id matches its argument to the active work record, so the halfword this
 * body reads from the step-entry table is that record id.
 * @source 0x8014DD3C
 */
extern void func_8014DD3C(u16 entry);

/* Shared main-executable resource request: it takes the requested resource id
 * and answers nothing, so the byte at +0x02 of the step-entry table is that id.
 * @source 0x8014D6B8
 */
extern void func_8014D6B8(u32 resource);

/* Entry 1 of this overlay's two-entry state-byte dispatch pair based at
 * 0x801F43EC; this body arms work byte +0x01 and then calls it, so it is the
 * next step of the same state sequence rather than an unknown callee.
 * @source 0x801F3B98
 */
extern void func_801F3B98(void);

/* @source 0x801F3A68
 * @behavior Entry step of this overlay's two-entry state-byte dispatch pair: it
 *           seeds the work record published through the scratchpad pointer cell
 *           0x1F800044 from the overlay's own record tables, applies that
 *           record's step entry through the shared main-executable helpers and
 *           then hands off to the pair's second entry.
 *
 *           It takes the record template selected by work byte +0x07 out of the
 *           0x98-byte record table of the main executable based at 0x80146888
 *           and copies its word triple +0x34/+0x38/+0x3C into the work record's
 *           own +0x34/+0x38/+0x3C, the position word pair and the position word
 *           with its bank tag that the neighbouring steps of this overlay move
 *           and publish. Each of the three copies re-reads the selector byte and
 *           re-forms the row address, so the index is spelled at each store. It
 *           then takes the 8-byte step entry that work byte +0x06 selects from
 *           the overlay table at 0x801F43D8 and hands that entry's unsigned
 *           halfword to the main lookup helper 0x8014DD3C as a record id, sets
 *           work byte +0x48 to 1, clears work bytes +0x24 and +0x2A, hands the
 *           entry's unsigned byte at +0x02 to the resource request 0x8014D6B8,
 *           and finally sets work byte +0x01 to 1 and calls 0x801F3B98, the
 *           handler the dispatcher on work byte +0x01 reaches at that index.
 *           Takes no arguments and returns nothing.
 *
 * Evidence: payload asm at 0x801F3A68 - the 0x18-byte frame keeps $ra at 0x14
 * and $s0 at 0x10; `lui $a0,0x1F80`/`lw $a0,0x44($a0)` loads the pointer cell
 * once and the three stores `sw $v0,0x34($a0)`, `sw $v0,0x38($a0)`,
 * `sw $v1,0x3C($a0)` follow `lbu $v1,0x7($a0)` re-reads against the compiler's
 * 0x98 form `sll 2 / addu / sll 2 / subu / sll 3` and `lui $at,%hi(SYM)` /
 * `addu $at,$at,$v0` / `lw %lo(SYM)($at)` at 0x801468BC, 0x801468C0 and
 * 0x801468C4. The calls are `lhu $a0` from 0x801F43D8 + 8 * (work byte +0x06)
 * into `jal 0x8014DD3C` with `addiu $s0,$zero,0x1` in its delay slot (the one
 * constant both later set bytes share, which is why $s0 is the only saved
 * register), then `sb $s0,0x48`, `sb $zero,0x24`, `sb $zero,0x2A`, each after a
 * fresh cell load, then `lbu $a0` from the same scaled entry +0x02 into
 * `jal 0x8014D6B8`, then a fresh cell load with `sb $s0,0x1($v0)` in the delay
 * slot of `jal 0x801F3B98`. The epilogue is lw $ra,0x14($sp); lw $s0,0x10($sp);
 * addiu $sp,$sp,0x18; jr $ra; nop, and the span ends on the last instruction it
 * claims, so it is one complete function with a single return.
 *
 * Name evidence: the position triple is the 0x34/0x38 pair plus the 0x3C word
 * the exact lifts of this target write as the position words
 * (stepWorkAccumulatorsByPhaseTableAndStepLevelTripleThenClaimRecordHandlerAc
 * World04Area19813_801F3388 moves the pair and publishFrontEndRecordIndexToWork
 * ByteBThenSeedHandlerA7RecordFromWorkPositionWorld04Area19813_801F3854 stores
 * the +0x1000000 address form of the word at 0x3C), the main lookup helper and
 * the resource request are the roles the shared main-executable lifts record at
 * 0x8014DD3C and 0x8014D6B8, and the state byte +0x01 hand-off is the local
 * dispatcher dispatchStateByte1Table43ecWorld04Area19813_801F3A24, whose table
 * at 0x801F43EC holds this body as entry 0 and 0x801F3B98 as entry 1. No `jal`
 * in the payload names 0x801F3A68; the only in-image reference is the word
 * 0x801F3A68 at 0x801F43EC, so the body is reached from outside its own code
 * spans rather than by call.
 *
 * Representation notes: both tables carry their stride in their own declaration
 * because the addresses' own scale is the stride, the 0x98-byte row and the
 * 8-byte step entry. Declaring them as flat byte arrays with the scaled byte
 * offset written in the subscript instead measured 70 of 76 instructions at a
 * 0x30-byte frame, first difference the prologue's own `addiu sp,sp,-40` with
 * every other row equal, so the surplus reservation sat in the two indexed
 * declarations and not in a statement: typing the step-entry table alone and
 * leaving the row table flat held the same 70 of 76, and typing both reached
 * 76 of 76 at this 0x18-byte frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkPositionTripleFromByte7RecordApplyLookupRecordAndRequestResourceThenArmState1World04Area19813_801F3A68(
    void) {
  u8* work;

  work = D_1F800044;
  *(u32*)(work + 0x34) = D_801468BC[(u32)work[7]].word_34;
  *(u32*)(work + 0x38) = D_801468BC[(u32)work[7]].word_38;
  *(u32*)(work + 0x3C) = D_801468BC[(u32)work[7]].word_3C;
  func_8014DD3C(D_801F43D8[(u32)D_1F800044[6]].entry_00);
  D_1F800044[0x48] = 1;
  D_1F800044[0x24] = 0;
  D_1F800044[0x2A] = 0;
  func_8014D6B8(D_801F43D8[(u32)D_1F800044[6]].resource_02);
  D_1F800044[1] = 1;
  func_801F3B98();
}

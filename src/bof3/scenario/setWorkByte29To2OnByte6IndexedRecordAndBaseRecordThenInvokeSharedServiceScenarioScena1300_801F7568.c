#include "bof3/bof3.h"

/* Shared work-record table at 0x80146888: 30 records of 0x98 bytes in main RAM,
 * outside this overlay's image. This target's generated undefined_syms_auto.txt
 * binds the address, so no target-map row is needed. */
extern u8 D_80146888[]; /* @source 0x80146888 @kind data */

/* Shared record base at 0x80145E90 in main RAM, also outside this overlay's
 * image and bound by this target's generated undefined_syms_auto.txt. The role
 * of the single record published from this base is not established by this
 * body, so the name stays address-anchored. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind data */

/* Shared main-executable service that consumes the record published through the
 * scratchpad pointer cell. Its callers disagree on the signature (see the note
 * in include/bof3/ui/game00_internal.h) and neither call site here sets an
 * argument register, so the declaration stays file-local and argument-free like
 * the other argument-free callers. */
extern void func_8014D290(void); /* @source 0x8014D290 @kind function */

/* Scratchpad work-object pointer cell 0x1F800044, shared by this overlay's
 * dispatchers and handlers. It is spelled as the named cell symbol rather than
 * as the bare constant slot: the constant-address form was measured keeping one
 * hoisted address in a saved register (`lui $s1,0x1F80` / `ori $s1,$s1,0x44`)
 * for every access, 148 bytes against the original 144 and first miss `ori
 * s1,s1,0x44` where the original loads `lw $s1,0x44($s1)`, while the original
 * re-materialises the cell address with its own `lui` before each store. The
 * same spelling measured exact in the world02/area085/13 lift
 * setWorkByte29To5OnActiveWorkRecordsAndBaseWorld02Area08513_801F2FCC, whose
 * tail stores this cell the same way, and a D_-prefixed address name binds at
 * its own address with no target-map row. */
extern u8* D_1F800044;

/* @source 0x801F7568
 * @behavior Two-step work-record publish of this overlay, called from four
 *           in-payload sites with no argument set and with any result
 *           discarded: 0x801F7288 inside
 *           armWorkCountdownAndAdvanceWorkByte1WhileSharedByteNotTwoThenInvokeTwoHelpersScenarioScena1300_801F7248,
 *           0x801F7344 inside the still-unlifted func_801F72A0, 0x801F742C
 *           inside the still-unlifted func_801F735C, and 0x801F746C inside
 *           resetWorkAreaAtProgress35ThenInvokeTwoHelpersScenarioScena1300_801F7444,
 *           which runs it after func_801F7484. It takes no arguments and
 *           returns nothing.
 *
 *           It first reads the scratchpad pointer cell 0x1F800044 once
 *           (`lui $s1,0x1F80` / `lw $s1,0x44($s1)`, scheduled between the
 *           frame's own saves) and keeps that incoming value in $s1. The cell
 *           holds the work object this overlay's dispatchers publish, and this
 *           body reads its byte 6 (`lbu $v1,0x6($s1)`) as an index into the
 *           shared 0x98-byte work-record table at 0x80146888: the
 *           `sll`/`addu`/`sll`/`subu`/`sll` chain gives 0x98 per index and the
 *           table base is materialised once into $v1, matching the stride and
 *           base that the exact world02/area085/13 lift
 *           setWorkByte29To5OnActiveWorkRecordsAndBaseWorld02Area08513_801F2FCC
 *           indexes with its loop counter and that the exact same-target lift
 *           convergeWorkRecordPairYOverCountdownThenAdvanceWorkByte1ScenarioScena1300_801F7958
 *           indexes with bytes 3 and 4 of this same object. The computed record
 *           is published through the cell (`lui $at,0x1F80` /
 *           `sw $v0,0x44($at)`), its state byte 0x29 receives the literal 2 -
 *           hoisted once into $s0 and reused for the second store - and the
 *           shared service at 0x8014D290 runs with the record published; the
 *           `sb` lands in that `jal` delay slot.
 *
 *           The second step publishes the base record at 0x80145E90
 *           unconditionally, stores 2 into that base's byte 0x29 through its own
 *           absolute address 0x80145EB9 (`lui $at,0x8014` / `sb $s0,0x5EB9($at)`,
 *           so no register holds the base for that store), and runs
 *           the same shared service once more, this time with a nop delay slot.
 *           It then restores the incoming cell value and returns. The 0x20-byte
 *           frame keeps $s1 at 0x14, $ra at 0x18 and $s0 at 0x10 and nothing
 *           else. The roles of work-object byte 6, of the record state byte
 *           0x29 and of the base record at 0x80145E90 are not established here,
 *           so the body is named for the record it selects and the value it
 *           stores.
 *
 * Statement order, operand forms, offset-local spelling and declaration style
 * come from the exact lift of the world/area sibling named above, whose tail is
 * the same base-record publish, byte-0x29 store and shared-service call; the
 * differences are this span's own: the literal 2 instead of 5, the record
 * selected by work-object byte 6 instead of a 30-record walk, and the incoming
 * cell value kept in $s1 instead of $s3. One payload scan over out/binaries
 * found the 144 bytes at offset 0x968 of this payload alone, so no
 * byte-identical twin exists anywhere in the tree and the sibling above is
 * shape evidence only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte29To2OnByte6IndexedRecordAndBaseRecordThenInvokeSharedServiceScenarioScena1300_801F7568(
    void) {
  u8 *saved;
  u8 *record;

  saved = D_1F800044;

  record = D_80146888 + saved[6] * 0x98;
  D_1F800044 = record;
  record[0x29] = 2;
  func_8014D290();

  D_1F800044 = D_80145E90;
  D_80145E90[0x29] = 2;
  func_8014D290();

  D_1F800044 = saved;
}

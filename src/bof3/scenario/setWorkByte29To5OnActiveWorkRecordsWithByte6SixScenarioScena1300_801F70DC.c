#include "bof3/bof3.h"

/* Shared work-record table at 0x80146888: 30 records of 0x98 bytes in main RAM,
 * outside this overlay's image. This target's generated undefined_syms_auto.txt
 * binds the address, so no target-map row is needed. */
extern u8 D_80146888[]; /* @source 0x80146888 @kind data */

/* Shared main-executable service that consumes the record published through the
 * scratchpad pointer cell. Its callers disagree on the signature (see the note
 * in include/bof3/ui/game00_internal.h) and this call site sets no argument
 * register, so the declaration stays file-local and argument-free like the other
 * argument-free callers. */
extern void func_8014D290(void); /* @source 0x8014D290 @kind function */

/* @source 0x801F70DC
 * @behavior Per-frame sweep of the shared work records, run from
 *           invokeTwoHelpersScenarioScena1300_801F6FD0 (0x801F6FD0) and from the
 *           byte-1 step at 0x801F6F24, which call it with no arguments after the
 *           overlay helper func_801F6FF8 and consume no result. It first reads
 *           the scratchpad pointer cell 0x1F800044 once (`lui $s1,0x1F80` /
 *           `lw $s1,0x44($s1)`) and keeps that value in a saved register. It
 *           then walks the 30 shared 0x98-byte work records based at 0x80146888
 *           in main RAM, outside this overlay's image: the loop counter is a
 *           byte-width quantity, so it is masked with 0xFF both for the scaled
 *           address (`sll`/`addu`/`sll`/`subu`/`sll` chain giving 0x98 per
 *           record) and for the 0x1E bound compared with `sltiu`. Each record is
 *           published through the cell (`lui $at,0x1F80` / `sw $v1,0x44($at)`)
 *           before it is tested through the same register. A record takes the
 *           dispatch only when its byte 0 has bit 0 set (`lbu` / `andi 0x1` /
 *           `beqz`) and its byte 6 reads 6 (`lbu` / `bne` against the hoisted
 *           6): that arm stores 5 into the record's byte 0x29 and runs the shared
 *           service at 0x8014D290 with no argument set, the `sb` of the hoisted
 *           literal landing in the `jal` delay slot. After the walk the incoming
 *           cell value is stored back (`lui $at,0x1F80` / `sw $s1,0x44($at)`).
 *           The 0x28-byte frame keeps $s0 to $s3 and $ra at 0x10 to 0x20 and
 *           nothing else. The roles of record byte 6, of the state byte 0x29 and
 *           of the shared service are not established here, so the loop is named
 *           for the values it stores and tests.
 *
 * Statement order, operand forms and the record walk come from the world02/area085/13
 * lift setWorkByte29To5OnActiveWorkRecordsAndBaseWorld02Area08513_801F2FCC
 * (0x801F2FCC), whose body was measured exact over the same 0x80146888 table,
 * stride, counter masking, scratchpad cell, byte-0 active test, 5 into byte 0x29
 * and the same 0x8014D290 call, differing only by this span's byte 6 == 6 test
 * and by a base-record store this span does not have. That discriminating
 * immediate is this span's own: one payload scan over out/binaries found the 188
 * bytes at offset 0x4DC in this payload alone, so no byte-identical twin exists.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte29To5OnActiveWorkRecordsWithByte6SixScenarioScena1300_801F70DC(
    void) {
  u8 *saved;
  u8 *record;
  u32 i;

  saved = PSX_REF(u8 *, SPAD_BASE + 0x44u);

  for (i = 0; (u8)i < 0x1E; i++) {
    record = D_80146888 + (u8)i * 0x98;
    PSX_REF(u8 *, SPAD_BASE + 0x44u) = record;

    if ((record[0] & 1) && (record[6] == 6)) {
      record[0x29] = 5;
      func_8014D290();
    }
  }

  PSX_REF(u8 *, SPAD_BASE + 0x44u) = saved;
}

#include "bof3/bof3.h"

/* Shared work-record table at 0x80146888: 30 records of 0x98 bytes in main RAM,
 * outside this overlay's image. Splat binds the address through this target's
 * generated undefined_syms_auto.txt, so no target-map row is needed. */
extern u8 D_80146888[]; /* @source 0x80146888 @kind data */

/* Shared work-record base at 0x80145E90, also outside this overlay's image. The
 * nearest sibling body in the world/area overlay family (func_801F3C84 of
 * emi/world02/area108/14) publishes records from this same base through the
 * same scratchpad cell, but with a 0x140-byte stride and a count byte; the role
 * of the single record published here is not further evidenced, so the name
 * stays address-anchored. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind data */

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * handlers. It is a named cell symbol, not the bare constant scratchpad slot:
 * the constant-address form is CSE'd into one saved register across the tail
 * call (measured 48/52 instructions, first miss `lui s0,0x1f80`), while the
 * original re-materialises the cell address with its own `lui at,0x1F80`
 * before each of the two tail stores. */
extern u8* D_1F800044;

/* Shared main-executable service that consumes the record published through the
 * scratchpad pointer cell. Its callers disagree on the signature (see the note
 * in include/bof3/ui/game00_internal.h), and neither call site in this body
 * sets an argument register, so the declaration stays file-local and
 * argument-free like the other argument-free callers. */
extern void func_8014D290(void); /* @source 0x8014D290 @kind function */

/* @source 0x801F2FCC
 * @behavior Overlay work-record teardown, the second half of
 *           invokeTwoHelpersWorld02Area08513_801F2EB0: walks the 30 shared
 *           0x98-byte work records based at 0x80146888, publishes each record
 *           through the scratchpad pointer cell 0x1F800044 and, for a record
 *           whose byte 0 has bit 0 set, stores 5 in that record's state byte
 *           0x29 and runs the shared service at 0x8014D290. It then publishes
 *           the base record at 0x80145E90 unconditionally, stores 5 in that
 *           base's byte 0x29, runs the shared service once more and restores
 *           the pointer cell's incoming value. Takes no arguments, returns
 *           nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the incoming cell value is loaded once
 * into $s3 before the frame is finished, the record index is a byte-width
 * quantity, so the loop counter is masked with 0xFF both for the address and
 * for the 0x1E bound and the masked value is carried in $v0 from the latch into
 * the next iteration's scaled index. The record pointer stays in $v1 and the
 * literal 5 is hoisted to $s1 before the loop. The 0x29 store of the base
 * record goes to its own absolute address (`lui at,0x8014`) because the base is
 * a fixed cell, not an indexed one.
 */
void setWorkByte29To5OnActiveWorkRecordsAndBaseWorld02Area08513_801F2FCC(
    void) {
  u8* saved;
  u8* record;
  u32 i;

  saved = D_1F800044;

  for (i = 0; (u8)i < 0x1E; i++) {
    record = D_80146888 + (u8)i * 0x98;
    D_1F800044 = record;

    if (record[0] & 1) {
      record[0x29] = 5;
      func_8014D290();
    }
  }

  D_1F800044 = D_80145E90;
  D_80145E90[0x29] = 5;
  func_8014D290();
  D_1F800044 = saved;
}

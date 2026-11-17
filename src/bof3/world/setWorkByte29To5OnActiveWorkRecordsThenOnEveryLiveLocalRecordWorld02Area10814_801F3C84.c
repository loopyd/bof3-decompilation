#include "bof3/bof3.h"

/* Shared work-record table at 0x80146888: 30 records of 0x98 bytes in main RAM,
 * outside this overlay's image. This target's generated undefined_syms_auto.txt
 * binds the address, so no target-map row is needed. */
extern u8 D_80146888[]; /* @source 0x80146888 @kind data */

/* Shared record array at 0x80145E90 in main RAM, also outside this overlay's
 * image and bound by this target's generated undefined_syms_auto.txt. This body
 * walks it with the live count below and a 0x140-byte stride, so the records
 * are live local records; the roles of their individual fields are not
 * established here and the name stays address-anchored. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind data */

/* Shared live-record count byte at 0x80146254. The walk re-reads it for every
 * loop test, and it is also the guard that skips the second walk entirely. */
extern u8 D_80146254; /* @source 0x80146254 @kind unknown */

/* Scratchpad work-object pointer cell 0x1F800044, shared by this overlay's
 * dispatchers and handlers and already named by this target's map. The incoming
 * value is loaded once into a saved register before the walks and stored back
 * after them, and every intermediate store re-materialises the cell address
 * with its own `lui $at,0x1F80`, which is what a named cell symbol emits. */
extern u8* workRecord44World02Area10814; /* @source 0x1F800044 @kind data */

/* Shared main-executable service that consumes the record published through the
 * scratchpad pointer cell. Its callers disagree on the signature (see the note
 * in include/bof3/ui/game00_internal.h) and neither call site in this body sets
 * an argument register, so the declaration stays file-local and argument-free
 * like the other argument-free callers. */
extern void func_8014D290(void); /* @source 0x8014D290 @kind function */

/* @source 0x801F3C84
 * @behavior Overlay work-record sweep, called with no arguments and with no
 *           result consumed by the byte-1 steps at 0x801F3920, 0x801F3A14 and
 *           0x801F3A6C. It reads the scratchpad pointer cell 0x1F800044 once and
 *           keeps the incoming value. It then walks the 30 shared 0x98-byte
 *           work records based at 0x80146888: each record is published through
 *           the cell and, when its byte 0 has bit 0 set, its state byte 0x29
 *           receives 5 and the shared service at 0x8014D290 runs with no
 *           argument set. A second walk covers the live local records counted
 *           by the shared byte 0x80146254, 0x140 bytes apart from the base
 *           0x80145E90: every one of them is published through the cell, takes 5
 *           in its byte 0x29 and runs the same service with no argument set, and
 *           the walk is skipped when that count byte reads zero. Both walks index
 *           with a byte-width counter masked with 0xFF, for the scaled address
 *           and for the loop test, and the cell value read on entry is restored
 *           last. Takes no arguments, returns nothing and keeps the 0x28-byte
 *           frame holding $s0 to $s3 and $ra at 0x10 to 0x20.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Statement order, operand forms and the first walk come from the exact
 * world02/area085/13 lift
 * setWorkByte29To5OnActiveWorkRecordsAndBaseWorld02Area08513_801F2FCC, whose
 * payload span in out/binaries is byte-identical to this one over its first
 * 0x88 bytes: same 0x80146888 table, 0x98 stride, counter masking, up-front
 * cell load into a saved register, byte-0 active test, 5 into byte 0x29, the
 * `sb` of that literal in the `jal` delay slot and the same 0x8014D290 call.
 * That comparison is also the clone evidence: the two spans first differ at
 * offset 0x88, exactly where the sibling publishes one base record and this
 * body opens the second walk, so no byte-identical twin exists. The guarded
 * second-walk tail and the closing restore are transcribed from the exact
 * world04/area172/13 lift
 * publishWorkRecordsWithOtIndex2AndRunSharedHandlerWorld04Area17213_801F2ED8,
 * which drives the same count byte, the same 0x140 stride and the same
 * publish/store/call body; this span carries no per-record filter, so its walk
 * publishes every live record. That sibling also carries the second walk in its
 * own pointer local, and the same holds here: sharing the first walk's pointer
 * local measured 64 of 67 instructions at the same 268-byte size, first
 * difference the closing `addu v1,v0,s2` that put the record in $v1 where the
 * original keeps it in $v0, while a local of its own matched on the first
 * measurement. Loop 2's literal 5 is re-materialised rather than carried over
 * from the first walk, and the count byte is re-read for every test.
 */
void setWorkByte29To5OnActiveWorkRecordsThenOnEveryLiveLocalRecordWorld02Area10814_801F3C84(
    void) {
  u8* saved;
  u8* record;
  u8* local;
  u32 i;

  saved = workRecord44World02Area10814;

  for (i = 0; (i & 0xFF) < 0x1E; i++) {
    record = D_80146888 + (i & 0xFF) * 0x98;
    workRecord44World02Area10814 = record;

    if (record[0] & 1) {
      record[0x29] = 5;
      func_8014D290();
    }
  }

  i = 0;
  if (D_80146254 != 0) {
    do {
      local = D_80145E90 + (i & 0xFF) * 0x140;
      workRecord44World02Area10814 = local;
      local[0x29] = 5;
      func_8014D290();
      i++;
    } while ((u32)(i & 0xFF) < D_80146254);
  }

  workRecord44World02Area10814 = saved;
}

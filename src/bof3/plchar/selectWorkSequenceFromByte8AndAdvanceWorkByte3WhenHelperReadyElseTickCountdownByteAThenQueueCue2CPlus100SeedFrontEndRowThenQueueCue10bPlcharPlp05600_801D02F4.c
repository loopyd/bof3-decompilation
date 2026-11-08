#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell before every access that follows a call or a
 * store that may alias the cell itself, so the source keeps the named-symbol
 * form and lets each access read the cell again.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8. This body addresses two row fields through their
 * own absolute symbols, because the original re-materialises each field base
 * and adds the scaled row offset again instead of keeping one row base.
 * @source 0x80143FD0 @kind unknown
 */
extern u8 D_80143FD0[];
/* Row field of the same 0x74-byte front-end row as D_80143FD0, two bytes on.
 * @source 0x80143FD2 @kind unknown
 */
extern u8 D_80143FD2[];

/* Readiness poll helper of the main executable; the sibling lifts of this
 * target and of the battle and bmagic overlays declare the same prototype. The
 * original keeps only the returned byte as the ready test.
 */
extern u8 func_8014DAEC(void);

/* Work-sequence selector of the shared EXE image. The step handler at
 * 0x801D0288 of this target and the sibling step handlers at 0x801D0060
 * (PLP016) and 0x801D0484 (PLP026) declare the same prototype and pass a
 * sequence byte.
 */
extern u32 func_8014D8D4(u8 sequence);

/* Cue dispatcher of the shared EXE image. The exact sibling lift at 0x801CF91C
 * declares the same prototype and passes the same kind of cue id.
 */
extern void func_8015DF18(u32 cue);

/* @source 0x801D02F4
 * @behavior Work byte 3 == 1 step of this overlay: entry 1 of the two-entry
 *           work-byte-3 handler run based at 0x801D08B0, whose entry 0 is
 *           0x801D0288. That base is word 22 of the twenty-four-entry pointer
 *           run the Splat `textbin` row at 0x801D0858 places, and the byte-3
 *           dispatcher at 0x801D0244 reaches it by scaling work byte 3, so this
 *           body has no direct caller of its own. It first calls the shared
 *           readiness helper func_8014DAEC and keeps only that answer's low
 *           byte. While the helper reports ready the step passes the work
 *           record's unsigned byte 8 to the shared work-sequence selector
 *           func_8014D8D4 and writes 2 into work byte 3, the value the sibling
 *           overlay step-1 handlers also write; the roles of work-byte-3 values
 *           past the two-entry run are not established here, so that store stays
 *           described by the value it writes. Otherwise the step reads work byte
 *           0x0A, the counter the previous step at 0x801D0288 arms with 0xB, and
 *           while it reads zero the step leaves every record field untouched. A
 *           nonzero counter is decremented and its new value is compared with
 *           zero again through a second read of the same work byte; only a
 *           countdown that reaches zero continues. That body queues the cue
 *           computed from the work record's unsigned halfword 0x2C plus 0x100
 *           through the shared cue dispatcher func_8015DF18, then copies work
 *           byte 8 into field 0x08 of the front-end row selected by work byte
 *           0x0B, whose row stride is 0x74, and copies work byte 7 into the same
 *           row's field 0x0A after defaulting work byte 7 to 1 when it reads
 *           zero. It closes by queueing the literal cue 0x10B through the same
 *           dispatcher. The roles of work byte 7, of row fields 0x08 and 0x0A,
 *           of the two cues and of the ready answer are not established, so
 *           those stay described by their address or by the effect the step has
 *           on them. The original scales the row index with the
 *           sll/subu/sll/addu/sll chain of a multiply by 0x74 and addresses both
 *           row stores through the field symbols, and it reloads the pointer
 *           cell before every access that follows a call or a store. The
 *           0x18-byte frame only keeps $ra at 0x10 across the calls. Its
 *           0x138-byte body differs from the shape siblings at 0x801D00CC of
 *           PLP016 and 0x801D04F0 of PLP026 in exactly one word, the `j` to the
 *           shared exit label at 0x801D0334, whose 26-bit target is the only
 *           relocation those positions change.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectWorkSequenceFromByte8AndAdvanceWorkByte3WhenHelperReadyElseTickCountdownByteAThenQueueCue2CPlus100SeedFrontEndRowThenQueueCue10bPlcharPlp05600_801D02F4(
    void) {
  if (func_8014DAEC()) {
    func_8014D8D4(D_1F800044[8]);
    D_1F800044[3] = 2;
  } else if (D_1F800044[0x0A] != 0) {
    D_1F800044[0x0A]--;
    if (D_1F800044[0x0A] == 0) {
      func_8015DF18(*(u16*)(D_1F800044 + 0x2C) + 0x100);
      D_80143FD0[(u32)D_1F800044[0x0B] * 0x74u] = D_1F800044[8];
      if (D_1F800044[7] == 0) {
        D_1F800044[7] = 1;
      }
      D_80143FD2[(u32)D_1F800044[0x0B] * 0x74u] = D_1F800044[7];
      func_8015DF18(0x10B);
    }
  }
}

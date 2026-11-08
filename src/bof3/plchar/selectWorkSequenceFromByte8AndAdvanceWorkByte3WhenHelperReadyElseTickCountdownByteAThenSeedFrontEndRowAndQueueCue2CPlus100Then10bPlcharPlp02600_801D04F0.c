#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell after every call and after every store that may
 * alias the cell itself, so the source keeps the named-symbol form and lets
 * each access read the cell again.
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

/* Readiness poll helper of the main executable; this target's 0x801D0054 step
 * and the battle and bmagic lifts declare the same prototype. The original
 * keeps only the returned byte as the ready test.
 */
extern u8 func_8014DAEC(void);

/* Work-sequence selector of the shared EXE image; the exact step at 0x801D0484
 * of this target declares the same prototype and passes the same sequence byte.
 */
extern u32 func_8014D8D4(u8 sequence);

/* Cue dispatcher of the shared EXE image; the exact PLP015 lift at 0x801CF91C
 * declares the same prototype and passes the same kind of cue id.
 */
extern void func_8015DF18(u32 cue);

/* @source 0x801D04F0
 * @behavior Work byte 3 == 1 step of this overlay, the second handler of the
 *           work-byte-3 run based at 0x801D0AA8: it is the word 0x801D04F0 at
 *           0x801D0AAC inside the thirty-word code-pointer run at 0x801D0A54,
 *           which the dispatcher
 *           dispatchStateByte3Table0aa8PlcharPlp02600_801D0440 indexes from
 *           that base, so it takes no arguments, no jal names it, and any
 *           result is discarded. It first runs the shared readiness helper
 *           func_8014DAEC and keeps only that answer's low byte. While the
 *           helper reports ready the step passes the work record's unsigned
 *           byte 8 to the shared work-sequence selector func_8014D8D4 and
 *           writes 2 into work byte 3, which advances the dispatch to entry 2 of
 *           the same run, the 0x801D0628 handler. Otherwise the step reads work
 *           byte 0x0A, the counter the previous step at 0x801D0484 arms with
 *           0xB, and while it reads zero the step leaves every record field
 *           untouched. A nonzero counter is decremented and its new value
 *           compared with zero again through a second read of the same work
 *           byte; only a countdown that reaches zero continues. That body
 *           queues the cue computed from the work record's unsigned halfword
 *           0x2C plus 0x100 through the shared cue dispatcher func_8015DF18,
 *           then copies work byte 8 into field 0x08 of the front-end row
 *           selected by work byte 0x0B, whose row stride is 0x74, and copies
 *           work byte 7 into the same row's field 0x0A after defaulting work
 *           byte 7 to 1 when it reads zero. It closes by queueing the literal
 *           cue 0x10B through the same dispatcher. The roles of work byte 7, of
 *           row fields 0x08 and 0x0A, of the two cues and of the ready answer
 *           are not established, so those stay described by their address or by
 *           the effect the step has on them.
 *           Original bytes: the row index is scaled with the sll/subu/sll/addu/
 *           sll chain of a multiply by 0x74 and both row stores address their
 *           field base through %hi/%lo, re-materialised per store. The pointer
 *           cell is reloaded after the readiness call, after the counter store
 *           and before each of the three row accesses, seven loads in all, and
 *           the 0x18-byte frame only keeps $ra at 0x10 across its four calls. The body is the same shape as the exact PLP016
 *           lift
 *           selectWorkSequenceFromByte8AndAdvanceWorkByte3WhenHelperReadyElseTickCountdownByteAThenSeedFrontEndRowAndQueueCue2CPlus100Then10bPlcharPlp01600_801D00CC:
 *           a compare of the two 312-byte spans reports 310 bytes equal, with
 *           only bytes 64 and 65, the target word of the `j` at 0x801D0530,
 *           differing, so that lift is shape evidence and not a clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectWorkSequenceFromByte8AndAdvanceWorkByte3WhenHelperReadyElseTickCountdownByteAThenSeedFrontEndRowAndQueueCue2CPlus100Then10bPlcharPlp02600_801D04F0(
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

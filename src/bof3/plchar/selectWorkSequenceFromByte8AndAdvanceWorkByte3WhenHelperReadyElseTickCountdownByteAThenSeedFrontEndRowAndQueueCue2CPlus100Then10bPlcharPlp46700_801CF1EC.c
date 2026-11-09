#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-loads the cell after
 * every call and after every store that may alias the cell itself, so the
 * source keeps the named-cell form and lets each access read the cell again.
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

/* Readiness poll helper of the shared EXE image, outside this payload. The
 * original keeps only the low byte of its answer as the ready test, and this
 * target's step handlers 0x801CED50 and 0x801CFC8C declare the same prototype.
 */
u8 func_8014DAEC(void);

/* Work-sequence selector of the shared EXE image, outside this payload. The
 * step handler 0x801CF180 of this target declares the same prototype and passes
 * the same work-sequence byte.
 */
u32 func_8014D8D4(u8 arg0);

/* Cue dispatcher of the shared EXE image, outside this payload. The step
 * handlers of PLP012, PLP015, PLP025 and PLP247 declare the same prototype and
 * pass cue ids of the same kind: a work halfword plus 0x100 here, 0x10B here.
 */
void func_8015DF18(u32 arg0);

/* @source 0x801CF1EC
 * @behavior Work byte 3 == 1 step of this overlay: the body address appears
 *           once in the payload's bytes, at word 0x801D03E4, which is index 1
 *           of the work-byte-3 handler run based at 0x801D03E0, so no jal in
 *           this payload names it. The run is entered by the byte-3 dispatcher
 *           0x801CF13C, which scales work byte 3 by four. It first runs the
 *           shared readiness helper func_8014DAEC and keeps only the low byte
 *           of that answer as a ready test. While the helper reports ready the
 *           step passes the work record's unsigned byte 8 to the shared
 *           work-sequence selector func_8014D8D4 and writes 2 into work byte 3,
 *           which advances the dispatch to entry 2 of the same run, the
 *           0x801CF324 handler. Otherwise the step reads work byte 0x0A, the
 *           counter the previous step at 0x801CF180 arms with 0xB, and while
 *           that byte reads zero it leaves every record field untouched. A
 *           nonzero counter is decremented and its new value compared with zero
 *           again through a second read of the same work byte; only a countdown
 *           that reaches zero continues. That path queues the cue computed from
 *           the work record's unsigned halfword 0x2C plus 0x100 through the
 *           shared cue dispatcher func_8015DF18, then copies work byte 8 into
 *           field 0x08 of the front-end row selected by work byte 0x0B, whose
 *           row stride is 0x74, and copies work byte 7 into the same row's
 *           field 0x0A after defaulting work byte 7 to 1 when it reads zero. It
 *           closes by queueing the literal cue 0x10B through the same
 *           dispatcher. The roles of work byte 7, of row fields 0x08 and 0x0A,
 *           of the two cues and of the ready answer are not established, so
 *           those stay described by their address or by the effect the step has
 *           on them. The original scales the row index with the sll/subu/sll/
 *           addu/sll chain of a multiply by 0x74 and addresses both row stores
 *           through the field symbols, and it reloads the pointer cell before
 *           every access that follows a call or a store. The 0x18-byte frame
 *           only keeps $ra at 0x10 across the calls. The body is the same body
 *           as PLP267 0x801CF7C8: of 78 instruction words the two spans
 *           differed in one, the j-format epilogue target, whose field shifted
 *           by exactly the 0x5DC delta between the two spans.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectWorkSequenceFromByte8AndAdvanceWorkByte3WhenHelperReadyElseTickCountdownByteAThenSeedFrontEndRowAndQueueCue2CPlus100Then10bPlcharPlp46700_801CF1EC(
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

#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-loads the cell after
 * every call and after the counter store, so the source keeps the named-cell
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

/* Readiness poll helper of the shared EXE image, outside this payload. The
 * original keeps only the low byte of its answer as the ready test, and this
 * target's step handlers 0x801CF130 and 0x801D006C declare the same prototype.
 */
extern u8 func_8014DAEC(void);

/* Work-sequence selector of the shared EXE image, outside this payload. The
 * exact step at 0x801CF560 of this target declares the same prototype and
 * passes the same work-sequence byte.
 */
extern u32 func_8014D8D4(u8 sequence);

/* Cue dispatcher of the shared EXE image, outside this payload. It takes the
 * work halfword offset by 0x100 here and the literal 0x10B here.
 */
extern void func_8015DF18(u32 cue);

/* @source 0x801CF5CC
 * @behavior Step of the front-end work object published at the scratchpad
 *           pointer cell 0x1F800044. The code-pointer word 0x801CF5CC sits at
 *           0x801D07D0, entry 1 of the code-pointer run based at 0x801D07CC
 *           that the payload byte-3 dispatcher 0x801CF51C indexes with work
 *           byte 3, so the body takes no arguments and no jal in this payload
 *           names it. The preceding step at 0x801CF560 writes 1 into work byte
 *           3 and arms the countdown byte 0x0A with 0xB. The body first runs
 *           the shared readiness helper func_8014DAEC and keeps only the low
 *           byte of that answer as a ready test. While the helper reports ready
 *           it passes the work record unsigned byte 8 to the shared
 *           work-sequence selector func_8014D8D4 and writes 2 into work byte 3,
 *           which advances the dispatch to entry 2 of the same run, the
 *           0x801CF704 handler. Otherwise it reads work byte 0x0A and, while
 *           that byte reads zero, leaves every record field untouched. A
 *           nonzero counter is decremented and its new value compared with zero
 *           through a second read of the same work byte; only a countdown that
 *           reaches zero continues. That path queues the cue computed from the
 *           work record unsigned halfword 0x2C plus 0x100 through the shared
 *           cue dispatcher func_8015DF18, copies work byte 8 into field 0x08 of
 *           the front-end row selected by work byte 0x0B, whose row stride is
 *           0x74 over the table based at 0x80143FC8, defaults work byte 7 to 1
 *           when it reads zero and copies that byte into the same row field
 *           0x0A, then closes by queueing the literal cue 0x10B through the
 *           same dispatcher. The roles of work byte 7, of row fields 0x08 and
 *           0x0A, of the two cues and of the ready answer are not established,
 *           so those stay described by their address or by the effect the step
 *           has on them. The original scales the row index with the
 *           sll/subu/sll/addu/sll chain of a multiply by 0x74 and addresses
 *           both row stores through their own field symbols 0x80143FD0 and
 *           0x80143FD2, re-materialising each hi/lo pair per store, and it
 *           reloads the scratchpad pointer cell after every call and after the
 *           counter store. The 0x18-byte frame only keeps the return address at
 *           0x10 across its four calls. When this seed was written the lift at
 *           PLP267 0x801CF7C8 measured exact, and a compare of the two 312-byte
 *           spans reported 310 bytes equal, the two differing bytes being the
 *           internal j-format word at 0x801CF60C, whose target shifts by
 *           exactly the 0x1FC delta between the two spans; the PLP016 lift at
 *           0x801D00CC measured the same 310 of 312 bytes equal through that
 *           one word at the 0xB00 delta. Both are shape evidence, not clones,
 *           so this seed transcribes the PLP267 statement order and writes its
 *           own address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectWorkSequenceFromByte8AndAdvanceWorkByte3WhenHelperReadyElseTickCountdownByteAThenSeedFrontEndRowAndQueueCue2CPlus100Then10bPlcharPlp56700_801CF5CC(
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

#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original loads the cell once for the even-byte test and the first store, then
 * re-reads it after every call and for every later access, because a call or a
 * store through the pointer may alias the cell itself; the source keeps the
 * named-symbol form and evaluates it at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Neighbour probe of this payload, outside this span and still unlifted. It
 * takes no arguments, reads work-record byte 8 as the index of the two-word
 * neighbour offsets it adds to record words 0x34 and 0x38, and returns a byte
 * flag for what it finds at that neighbour: it answers through $v0 with the
 * 0/1 the original call sites test, so the declared return type is that byte.
 */
extern u8 func_801CFEAC(void);

/* Cue queue routine of the shared EXE image, outside this payload. The original
 * calls it through a plain `jal` with the work halfword 0x2C plus 0x100 in $a0,
 * computes that argument before the frame is created for the call, and the
 * added constant issues in the call's own delay slot. The exact sibling
 * queueCue2CPlus100ThenSelectWorkSequenceFromByte8ArmCountdownAndAdvanceWorkByte3PlcharPlp03400_801CFC7C
 * of this payload declares the same word argument and passes the same
 * expression.
 */
extern void func_8015DF18(u32 arg0);

/* Work-sequence selector of the shared EXE image, outside this payload. The
 * original passes the sequence ((work byte 8 - 1) / 2 + 0x42) truncated to a
 * byte, which keeps both divisions signed, and masks the value in the call's
 * own delay slot. The same payload sibling passes the same expression.
 */
extern u32 func_8014D8D4(u8 arg0);

/* @source 0x801CF3C0
 * @behavior Overlay step reached through the code-pointer word 0x801CF3C0 at
 *           0x801D0320, entry 11 of this overlay's twenty-eight-word handler run
 *           at 0x801D02F4 and index 0 of the run based at 0x801D0320 that the
 *           byte-3 dispatcher dispatchWorkByte3Table0320PlcharPlp03400_801CF37C
 *           indexes from that base with work-record byte 3 scaled by four, so
 *           the body runs as that dispatcher's step 0; no jal in this payload
 *           names it, so it is reached through the run alone. It first settles
 *           the work record's byte 8, the index the payload's own probe
 *           func_801CFEAC uses to pick the two-word neighbour offset it adds to
 *           record words 0x34 and 0x38. When that byte reads even the handler
 *           stores byte 8 minus one masked to three bits, calls the probe, and
 *           only while the probe's byte answer reads zero stores byte 8 plus
 *           two masked the same way, calls the probe once more, and again only
 *           while that answer reads zero stores that value minus two, which
 *           restores the minus-one candidate; so the stored index ends at the
 *           minus-one neighbour when the probe reports anything there, at the
 *           plus-one neighbour when only that one reports something, and at the
 *           minus-one neighbour again when neither reports anything. Every
 *           probe result is discarded beyond that test. The handler then runs
 *           the shared queue and sequence pair this payload's dispatch entry
 *           at 0x801CFC7C runs: it queues the cue taken from the record's
 *           unsigned halfword 0x2C plus 0x100 through func_8015DF18, selects
 *           the shared work sequence ((record byte 8 - 1) / 2 + 0x42) truncated
 *           to a byte through func_8014D8D4, clears the step-sign byte 0x0B and
 *           the front-end record selection byte 6, arms the countdown byte 0x0A
 *           with 8 and sets the dispatch bytes 3 to 1, so the next dispatch of
 *           the run reaches the entry at 0x801D0324. Takes no arguments and
 *           returns nothing, and the 0x18-byte frame only keeps the return
 *           address at 0x10 across its four calls. The pointer cell is loaded
 *           before the frame is created and re-read after each call and before
 *           every later access. The roles of byte 8 as a neighbour index and of
 *           the cleared cells follow the probe body and this payload's other
 *           dispatch entries; the game meaning of the even-byte guard is not
 *           established here, so the name describes the effect. The payload
 *           plp349/00 holds the same 0x120-byte body at 0x801CE698 with only
 *           its two probe-call words differing, which is relocation evidence
 *           for this statement order, and that row is still asm.
 * @status exact
 * @match 100.00
 * @residual none
 */
void probeWorkNeighbourIndexThenQueueCueSelectSequenceClearStepSignAndSelectionArmCountdownAndAdvanceWorkStepPlcharPlp03400_801CF3C0(
    void) {
  if ((D_1F800044[8] & 1) == 0) {
    D_1F800044[8] = (D_1F800044[8] - 1) & 7;
    if (!func_801CFEAC()) {
      D_1F800044[8] = (D_1F800044[8] + 2) & 7;
      if (!func_801CFEAC()) {
        D_1F800044[8] = (D_1F800044[8] - 2) & 7;
      }
    }
  }
  func_8015DF18(*(u16*)(D_1F800044 + 0x2C) + 0x100);
  func_8014D8D4((u8)((D_1F800044[8] - 1) / 2 + 0x42));
  D_1F800044[0xB] = 0;
  D_1F800044[6] = 0;
  D_1F800044[0xA] = 8;
  D_1F800044[3] = 1;
}

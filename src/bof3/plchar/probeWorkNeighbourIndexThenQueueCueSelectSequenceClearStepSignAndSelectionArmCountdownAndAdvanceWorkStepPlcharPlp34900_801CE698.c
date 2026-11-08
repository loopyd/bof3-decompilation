#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original loads the cell once for the even-byte test and the first store, then
 * re-reads it after each of the two probe calls and before every later access,
 * because a call or a store through the pointer may alias the cell itself; the
 * source keeps the named-symbol form and evaluates it at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Neighbour probe of this payload, now lifted at 0x801CF184 as
 * testProjectedWorkPositionAgainstWorkRecordsHelperOrMapCodesF0F1F4F6F7PlcharPlp34900_801CF184.
 * It reads work-record byte 8 as the index of the two direction-vector word
 * tables at 0x80181B94 and 0x80181B98, adds the selected word pair to the record
 * position held at work +0x34 and classifies the tile the pair lands on,
 * answering through $v0 with the byte the original call sites test, so the
 * declared return type is that unsigned byte and the caller owes no callee lift.
 */
u8 testProjectedWorkPositionAgainstWorkRecordsHelperOrMapCodesF0F1F4F6F7PlcharPlp34900_801CF184(
    void);

/* Cue queue routine of the shared EXE image, outside this payload. The original
 * calls it through a plain `jal` with the work halfword 0x2C plus 0x100 in $a0,
 * computes that argument before the frame is created for the call, and the added
 * constant issues in the call's own delay slot. Splat binds its address through
 * this target's generated undefined_funcs_auto.txt, and the lift at 0x801CEF54 of
 * this payload declares the same word argument and passes the same expression.
 */
void func_8015DF18(u32 arg0);

/* Work-sequence selector of the shared EXE image, outside this payload. The
 * original passes the sequence ((work byte 8 - 1) / 2 + 0x42) truncated to a
 * byte, which keeps both divisions signed, and masks the value in the call's own
 * delay slot. Splat binds its address through this target's generated
 * undefined_funcs_auto.txt, and the lifts at 0x801CEF54 and 0x801CEB24 of this
 * payload declare the same prototype.
 */
u32 func_8014D8D4(u8 arg0);

/* Seeded from the body at plp034/00 0x801CF3C0: the two payload spans are equal
 * in all 72 words except the two probe `jal` words at +0x28 and +0x58, which name
 * this payload's own probe at 0x801CF184, so that sibling is shape evidence and
 * this span is no clone of it. Statement order, the pointer-cell access form and
 * the declaration style are carried over with this function's own name and this
 * payload's own dispatch facts.
 */
/* @source 0x801CE698
 * @behavior Work byte 3 == 0 step of this overlay: this payload publishes the
 *           address at the pointer word 0x801CF600, entry 5 of the sixteen-word
 *           code-pointer run based at 0x801CF5EC, and that word is the base the
 *           work-byte-3 dispatcher
 *           dispatchWorkByte3Tablef600PlcharPlp34900_801CE654 indexes by work
 *           byte 3 scaled by four, so the dispatcher reaches this body with index
 *           zero; no `jal` in this payload names it, so it is reached through the
 *           run alone. When work byte 8 is even it first rewrites that byte: it
 *           stores the decremented direction ((byte 8 - 1) & 7), calls the
 *           payload neighbour probe
 *           testProjectedWorkPositionAgainstWorkRecordsHelperOrMapCodesF0F1F4F6F7PlcharPlp34900_801CF184
 *           and, only while that call reads
 *           zero, stores the neighbour two steps the other way ((byte 8 + 2) & 7)
 *           and calls the same probe again; if that answer reads zero too the byte
 *           is put back to the decremented direction, so the stored index ends at
 *           the decremented neighbour when the probe reports anything there, at
 *           the other neighbour when only that one reports something, and at the
 *           decremented one again when neither reports anything. Every probe
 *           result is discarded beyond that test. Either path then runs the
 *           shared queue and sequence pair of this payload's other dispatch steps:
 *           it queues the cue taken from the record's unsigned halfword 0x2C plus
 *           0x100 through func_8015DF18, selects the shared work sequence
 *           ((record byte 8 - 1) / 2 + 0x42) truncated to a byte through
 *           func_8014D8D4, clears the step-sign byte 0x0B and the front-end
 *           record selection byte 6, arms the countdown byte 0x0A with 8 and sets
 *           the dispatch byte 3 to 1, so the next dispatch of the same run reaches
 *           the entry at 0x801CF604. The subtraction leaves the division signed,
 *           so byte 8 == 0 and byte 8 == 1 both select sequence 0x42 while
 *           byte 8 == 3 selects 0x43. Takes no arguments and returns nothing; the
 *           0x18-byte frame only keeps the return address at 0x10 across its four
 *           calls, and the pointer cell is loaded before the frame is created and
 *           re-read after each call and before every later access. The roles of
 *           byte 8 as a neighbour index and of the two cleared cells follow the
 *           probe body and this payload's other dispatch entries; the game
 *           meaning of the even-byte guard and of the requested cue are not
 *           established here, so the name describes the effect. The 0x120-byte
 *           span at payload offset 0x298 sits exactly between the two declared
 *           rows that bound it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void probeWorkNeighbourIndexThenQueueCueSelectSequenceClearStepSignAndSelectionArmCountdownAndAdvanceWorkStepPlcharPlp34900_801CE698(
    void) {
  if ((D_1F800044[8] & 1) == 0) {
    D_1F800044[8] = (D_1F800044[8] - 1) & 7;
    if (!testProjectedWorkPositionAgainstWorkRecordsHelperOrMapCodesF0F1F4F6F7PlcharPlp34900_801CF184()) {
      D_1F800044[8] = (D_1F800044[8] + 2) & 7;
      if (!testProjectedWorkPositionAgainstWorkRecordsHelperOrMapCodesF0F1F4F6F7PlcharPlp34900_801CF184()) {
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

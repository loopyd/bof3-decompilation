#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-loads it before the
 * byte-8 read of every block and again before each of the three stores, so the
 * source keeps the named symbol form and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Direction step test of this payload: it reads work byte 8, takes that entry of
 * the direction-vector word tables at 0x80181B94/0x80181B98, offsets the record
 * position held at work +0x34/+0x38 by the entry and classifies the tile the step
 * lands on, returning one byte. Its own Splat row was still `asm` when this
 * caller was lifted, so this caller takes its result as an unsigned byte, and
 * the name below is the one that callee's own lift establishes.
 */
u8 testProjectedWorkPositionAgainstWorkRecordsHelperOrMapCodesF0F1F4F6F7PlcharPlp05600_801D04C8(
    void);

/* Layer refresh of this payload. The original calls it through a plain `jal`
 * with nothing set up in the argument registers, so it takes no arguments and
 * this caller reads no result from $v0. Its 0x164-byte body is byte-identical,
 * all 89 instructions, to the refresh the two sibling step handlers of this
 * payload call.
 */
void claimWorkByte2BThenClearItWhenRouteThreeOrFiveSampleAbove64AndBeyondWorkReferencePlcharPlp05600_801D066C(
    void);

/* Work-sequence selector of the shared EXE image, outside this payload. The
 * sibling step handlers at 0x801D0060 (PLP016) and 0x801D0484 (PLP026) declare
 * the same prototype and pass a sequence id, so the two-sequence call here
 * passes a small immediate. Splat binds the address through this target's
 * generated undefined_funcs_auto.txt.
 */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CF9DC
 * @behavior Work byte 3 == 0 step of this overlay: entry 0 of the work-byte-3
 *           handler run based at 0x801D089C, whose entry 1 is 0x801D02F4. That
 *           base is entry 17 of the twenty-four-entry pointer run the Splat
 *           `textbin` row at 0x801D0858 places, and the byte-3 dispatcher at
 *           0x801CF998 reaches it by scaling work byte 3, so this body has no
 *           direct caller of its own. When work byte 8 is even it first rewrites
 *           that byte: it stores the decremented direction ((byte 8 - 1) & 7),
 *           calls the payload step test at 0x801D04C8 and, only while that call
 *           reads zero, stores the neighbour two steps the other way
 *           ((byte 8 + 2) & 7) and calls the same test again; if that call reads
 *           zero too the byte is put back to the decremented direction. Both
 *           paths then run the payload layer refresh at 0x801D066C, select the
 *           shared work sequence through func_8014D8D4 with
 *           ((byte 8 - 1) / 2 + 0x42) truncated to a byte, clear work byte 0xB,
 *           write 0xB into work byte 0xA and 1 into work byte 3, which advances
 *           the dispatch to the next entry of the same run. The original
 *           re-loads the pointer cell before the byte-8 read of every block and
 *           before each of the three stores; the subtraction leaves the division
 *           signed, so byte 8 == 0 still selects sequence 0x42. It takes no
 *           arguments and returns nothing; the 0x18-byte frame only keeps $ra at
 *           0x10 across the calls. Its 0x100-byte body reappears in six other
 *           PLCHAR overlays (for example func_801CF7B4 of PLP016, which is
 *           exact, and func_801CFBD8 of PLP026) with different `jal` targets and
 *           the same frame, so those are shape evidence, not clones, and this
 *           body's own step test and refresh are 0x801D04C8 and 0x801D066C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void adjustEvenWorkByte8ToNeighbourWhenStepTestReadsZeroThenRefreshLayersSelectSequenceAndAdvanceWorkByte3PlcharPlp05600_801CF9DC(
    void) {
  if (!(D_1F800044[8] & 1)) {
    D_1F800044[8] = (D_1F800044[8] - 1) & 7;
    if (testProjectedWorkPositionAgainstWorkRecordsHelperOrMapCodesF0F1F4F6F7PlcharPlp05600_801D04C8() ==
        0) {
      D_1F800044[8] = (D_1F800044[8] + 2) & 7;
      if (testProjectedWorkPositionAgainstWorkRecordsHelperOrMapCodesF0F1F4F6F7PlcharPlp05600_801D04C8() ==
          0) {
        D_1F800044[8] = (D_1F800044[8] - 2) & 7;
      }
    }
  }
  claimWorkByte2BThenClearItWhenRouteThreeOrFiveSampleAbove64AndBeyondWorkReferencePlcharPlp05600_801D066C();
  func_8014D8D4((u8)((D_1F800044[8] - 1) / 2 + 0x42));
  D_1F800044[0xB] = 0;
  D_1F800044[0xA] = 0xB;
  D_1F800044[3] = 1;
}

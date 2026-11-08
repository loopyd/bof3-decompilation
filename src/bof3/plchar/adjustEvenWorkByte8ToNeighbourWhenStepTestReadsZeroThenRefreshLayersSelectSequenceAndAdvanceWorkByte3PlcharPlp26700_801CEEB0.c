#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-loads it before the
 * byte-8 read of every block and again before each of the three stores, so the
 * source keeps the named symbol form and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Direction step test of this payload: it reads work byte 8, takes that entry of
 * the direction-vector word tables at 0x80181B94/0x80181B98, offsets the record
 * position held at work +0x34 by the entry and classifies the tile the step lands
 * on, returning one byte. This caller took its result as an unsigned byte while
 * the step test row at 0x801CF99C was still `asm`; that row's own lift declares
 * the same prototype.
 */
u8 testProjectedWorkPositionAgainstWorkRecordsHelperOrMapCodesF0F1F4F6F7PlcharPlp26700_801CF99C(
    void);

/* Layer refresh of this payload. The original calls it through a plain `jal`
 * with nothing set up in the argument registers, so it takes no arguments and
 * this caller reads no result from $v0. Its 0x164-byte body writes 1 into work
 * byte 0x2B, probes the two layer tables through 0x80154F78/0x80154F28 against
 * the work reference halfword 0x3E and clears that byte again when a probe reads
 * above 64 and beyond the reference; its own row here is still `asm`, and the
 * exact sibling step handler at 0x801CF75C of this payload calls the same body.
 */
void func_801CFB40(void);

/* Work-sequence selector of the shared EXE image, outside this payload. The
 * sibling step handlers at 0x801D0060 (PLP016), 0x801D0288 (PLP056) and
 * 0x801D0484 (PLP026) declare the same prototype and pass a sequence id, so the
 * call here passes a small immediate. Splat binds the address through this
 * target's generated undefined_funcs_auto.txt, and the exact sibling at
 * 0x801CF75C of this payload declares the same prototype.
 */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CEEB0
 * @behavior Work byte 3 == 0 step of this overlay: the word this payload's
 *           thirty-entry code-pointer run at 0x801D0988 holds at 0x801D09B0,
 *           entry 10, and that word is the table base the work-byte-3 dispatcher
 *           at 0x801CEE6C indexes by work byte 3, so the dispatcher reaches this
 *           body with index zero and the body has no direct caller of its own.
 *           When work byte 8 is even it first rewrites that byte: it stores the
 *           decremented direction ((byte 8 - 1) & 7), calls the payload step test
 *           at 0x801CF99C and, only while that call reads zero, stores the
 *           neighbour two steps the other way ((byte 8 + 2) & 7) and calls the
 *           same test again; if that call reads zero too the byte is put back to
 *           the decremented direction. Both paths then run the payload layer
 *           refresh at 0x801CFB40, select the shared work sequence through
 *           func_8014D8D4 with ((byte 8 - 1) / 2 + 0x42) truncated to a byte,
 *           clear work byte 0xB, write 0xB into work byte 0xA and 1 into work
 *           byte 3, which advances the dispatch to entry 11 of the same run. The
 *           original re-loads the pointer cell before the byte-8 read of every
 *           block and before each of the three stores, and its odd-byte path
 *           skips the whole neighbour adjustment; the subtraction leaves the
 *           division signed, so byte 8 == 0 still selects sequence 0x42 while
 *           byte 8 == 1 and byte 8 == 2 both give 0x42 and byte 8 == 3 gives
 *           0x43. It takes no arguments and returns nothing; the 0x18-byte frame
 *           only keeps $ra at 0x10 across the calls. All 64 words of its
 *           0x100-byte body match func_801CF7B4 of PLP016 and func_801CF9DC of
 *           PLP056 except the three `jal` words at +0x28, +0x58 and +0x8C, which
 *           name this payload's step test and refresh, so those two are shape
 *           evidence with their own `jal` targets and this body is not a clone of
 *           either.
 * @status exact
 * @match 100.00
 * @residual none
 */
void adjustEvenWorkByte8ToNeighbourWhenStepTestReadsZeroThenRefreshLayersSelectSequenceAndAdvanceWorkByte3PlcharPlp26700_801CEEB0(
    void) {
  if (!(D_1F800044[8] & 1)) {
    D_1F800044[8] = (D_1F800044[8] - 1) & 7;
    if (testProjectedWorkPositionAgainstWorkRecordsHelperOrMapCodesF0F1F4F6F7PlcharPlp26700_801CF99C() ==
        0) {
      D_1F800044[8] = (D_1F800044[8] + 2) & 7;
      if (testProjectedWorkPositionAgainstWorkRecordsHelperOrMapCodesF0F1F4F6F7PlcharPlp26700_801CF99C() ==
          0) {
        D_1F800044[8] = (D_1F800044[8] - 2) & 7;
      }
    }
  }
  func_801CFB40();
  func_8014D8D4((u8)((D_1F800044[8] - 1) / 2 + 0x42));
  D_1F800044[0xB] = 0;
  D_1F800044[0xA] = 0xB;
  D_1F800044[3] = 1;
}

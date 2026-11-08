#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-loads it before the
 * record-byte read of every block and again before each of the three stores, so
 * the source keeps the named symbol form and evaluates it again at every
 * access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Step test of this payload, whose own Splat row is still `asm`: it reads work
 * byte 8, takes that entry of the direction-vector word tables at
 * 0x80181B94/0x80181B98, offsets the record position held at work +0x34/+0x38 by
 * the entry and classifies the tile the step lands on. This caller reads that
 * answer as an unsigned byte without a lift of the callee.
 */
u8 func_801CF7A0(void);

/* Position refresh of this payload, still an `asm` row: it claims work byte 0x2B,
 * offsets the work position held at +0x34/+0x38 by the deltas at
 * 0x80181BAC/0x80181BB0 and re-projects that position through func_80154F78. The
 * original calls it through a plain `jal` with nothing set up in the argument
 * registers, so it takes no arguments and this caller reads no result from $v0.
 */
void func_801CF944(void);

/* Work-sequence selector of the shared EXE image, outside this payload. Splat
 * binds the address through this target's generated undefined_funcs_auto.txt, and
 * the exact sibling at 0x801CFBD8 of the PLCHAR overlay plp026 declares the same
 * prototype.
 */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CECB4
 * @behavior Work byte 3 == 0 step of the second byte-3 dispatch group of this
 *           overlay: entry 0 of the handler run that the byte-3 dispatcher
 *           D_801D07B8 indexes, the word at 0x801D07B8 of the in-image pointer
 *           run at 0x801D078C. When work byte 8 is even it first rewrites that
 *           byte: it stores the decremented direction ((byte 8 - 1) & 7), calls
 *           the payload step test func_801CF7A0 and, only while that call reads
 *           zero, stores the neighbour two steps the other way ((byte 8 + 2) & 7)
 *           and calls the same test again; if that call reads zero too the byte is
 *           put back to the decremented direction. Both paths then run the
 *           payload refresh func_801CF944, select the shared work sequence
 *           through func_8014D8D4 with ((byte 8 - 1) / 2 + 0x42) truncated to a
 *           byte, clear work byte 0xB, write 0xB into work byte 0xA and 1 into
 *           work byte 3, which advances the dispatch to entry 1 of the same run,
 *           the handler at 0x801CEDB4. The original re-loads the pointer cell
 *           before the byte-8 read of every block and before each of the three
 *           stores; the subtraction leaves the division signed, so byte 8 == 0
 *           still selects sequence 0x42. It takes no arguments and returns
 *           nothing, and the 0x18-byte frame only keeps $ra at 0x10 across the
 *           calls. Its 0x100-byte body reappears in the PLCHAR overlays with
 *           other `jal` words and the same frame: compared over the original
 *           payload bytes, 253 of 256 bytes were equal to the now exact lift at
 *           0x801CFBD8 of plp026 and three words differed, the low immediate of
 *           the two step-test calls and of the refresh call, so that sibling was
 *           shape evidence and not a clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void adjustEvenWorkByte8ToNeighbourWhenStepTestReadsZeroThenRefreshSelectSequenceAndAdvanceWorkByte3PlcharPlp56700_801CECB4(
    void) {
  if (!(D_1F800044[8] & 1)) {
    D_1F800044[8] = (D_1F800044[8] - 1) & 7;
    if (func_801CF7A0() == 0) {
      D_1F800044[8] = (D_1F800044[8] + 2) & 7;
      if (func_801CF7A0() == 0) {
        D_1F800044[8] = (D_1F800044[8] - 2) & 7;
      }
    }
  }
  func_801CF944();
  func_8014D8D4((u8)((D_1F800044[8] - 1) / 2 + 0x42));
  D_1F800044[0xB] = 0;
  D_1F800044[0xA] = 0xB;
  D_1F800044[3] = 1;
}

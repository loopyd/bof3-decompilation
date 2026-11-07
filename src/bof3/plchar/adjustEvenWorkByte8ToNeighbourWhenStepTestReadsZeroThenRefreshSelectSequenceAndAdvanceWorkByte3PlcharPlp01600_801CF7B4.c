#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-loads it before the
 * record-byte read of every block and again before each store, so the source
 * keeps the named symbol form and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Direction step test of this payload: it reads work byte 8, takes that entry of
 * the direction-vector word tables at 0x80181B94/0x80181B98, offsets the record
 * position held at work +0x34/+0x38 by the entry and classifies the tile the step
 * lands on, returning one byte. Its own Splat row is still `asm`, so this caller
 * takes its result as an unsigned byte without a lift of the callee.
 */
u8 func_801D02A0(void);

/* Two-layer position refresh of this payload; the original calls it through a
 * plain `jal` with nothing set up in the argument registers, so it takes no
 * arguments. Its own Splat row is still `asm`.
 */
void func_801D0444(void);

/* Work-sequence selector of the shared EXE image, outside this payload. Splat
 * binds the address through this target's generated undefined_funcs_auto.txt, and
 * the exact sibling at 0x801D0060 of this payload declares the same prototype.
 */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CF7B4
 * @behavior Work byte 3 == 0 step of the second dispatch group of this overlay:
 *           entry 16 of the thirty-word code-pointer run at 0x801D0630, whose
 *           base word sits at 0x801D0670 and is what the work-byte-3 dispatcher
 *           0x801CF770 indexes, so this handler runs for work byte 3 == 0. When
 *           work byte 8 is even it first rewrites that byte: it stores the
 *           decremented direction ((byte 8 - 1) & 7), calls the payload step test
 *           func_801D02A0 and, only while that call reads zero, stores the
 *           neighbour two steps the other way ((byte 8 + 2) & 7) and calls the
 *           same test again; if that call reads zero too the byte is put back to
 *           the decremented direction. Both paths then run the payload refresh
 *           func_801D0444, select the shared work sequence through func_8014D8D4
 *           with ((byte 8 - 1) / 2 + 0x42) truncated to a byte, clear work byte
 *           0xB, write 0xB into work byte 0xA and 1 into work byte 3, which
 *           advances the dispatch to the next entry of the same run. The original
 *           re-loads the pointer cell before the byte-8 read of every block and
 *           before each of the three stores; the subtraction leaves the division
 *           signed, so byte 8 == 0 still selects sequence 0x42. It takes no
 *           arguments and returns nothing; the 0x18-byte frame only keeps $ra at
 *           0x10 across the calls. Its 0x100-byte body reappears in six other
 *           PLCHAR overlays (for example func_801CFBD8 of PLP026 and
 *           func_801CE8D4 of PLP467) with different `jal` targets and the same
 *           frame, so those are shape evidence, not clones, and their own rows
 *           are still `asm`.
 * @status exact
 * @match 100.00
 * @residual none
 */
void adjustEvenWorkByte8ToNeighbourWhenStepTestReadsZeroThenRefreshSelectSequenceAndAdvanceWorkByte3PlcharPlp01600_801CF7B4(
    void) {
  if (!(D_1F800044[8] & 1)) {
    D_1F800044[8] = (D_1F800044[8] - 1) & 7;
    if (func_801D02A0() == 0) {
      D_1F800044[8] = (D_1F800044[8] + 2) & 7;
      if (func_801D02A0() == 0) {
        D_1F800044[8] = (D_1F800044[8] - 2) & 7;
      }
    }
  }
  func_801D0444();
  func_8014D8D4((u8)((D_1F800044[8] - 1) / 2 + 0x42));
  D_1F800044[0xB] = 0;
  D_1F800044[0xA] = 0xB;
  D_1F800044[3] = 1;
}

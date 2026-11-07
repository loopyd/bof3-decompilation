#include "bof3/bof3.h"

/* The three 16.16 panel accumulators driven by func_8019AA44 (0x8019AA44) of
 * emi/etc/game/00; that update adds the per-frame deltas below and writes the
 * integer part back to the signed halfword panel coordinates. */
extern s32 D_801481E8;
extern s32 D_801481EC;
extern s32 D_801481F0;

/* The paired per-frame deltas the same update adds to the accumulators. */
extern s32 D_801481F8;
extern s32 D_801481FC;
extern s32 D_80148200;

/* Signed halfword panel coordinates the accumulators are reseeded from. */
extern s16 D_801492D8;
extern s16 D_801492DA;
extern s16 D_801492DC;

/* Shared main-RAM mode byte, bound by config/targets/shared/symbols.txt. */
extern u8 D_80149333;

/* @source 0x801F7104
 * @behavior Reseeds the three 16.16 panel accumulators D_801481E8 /
 * D_801481EC / D_801481F0 with the integer part of the signed halfword panel
 * coordinates D_801492D8 / D_801492DA / D_801492DC (each coordinate shifted
 * left sixteen), zeroes the paired per-frame deltas D_801481F8 / D_801481FC /
 * D_80148200, then reads the work-record byte at offset 9 through the
 * scratchpad pointer cell 0x1F800044 and publishes it in the shared mode byte
 * D_80149333, and finally selects handler 1 by storing 1 in that work record's
 * byte at offset 1, which is what $v0 holds as 1 at the return. The pointer
 * cell is loaded once and reused for both work-record accesses. The 116 bytes
 * at file offset 0x504 are the payload's second code word and no in-image jal
 * or pointer word names them, so the overlay loader enters the function from
 * outside this payload; the only in-image reference to 0x801F7104 is the
 * second word of the overlay handler run D_801FAEC0, whose dispatcher indexes
 * the run from its base.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedPanelAccumulatorsThenPublishWorkByte9AndSelectHandler1ScenarioScena1100_801F7104(void) {
  u8* work;

  D_801481E8 = D_801492D8 << 16;
  D_801481EC = D_801492DA << 16;
  D_801481F0 = D_801492DC << 16;
  D_801481F8 = 0;
  D_801481FC = 0;
  D_80148200 = 0;
  work = SPAD_PTR_SLOT(u8, 0x44u);
  D_80149333 = work[9];
  work[1] = 1;
}

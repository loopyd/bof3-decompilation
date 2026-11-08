#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

void drawFullScreenRedGradientBandCenteredOnScreenCenterScenarioSce15ef200_801D19A8(
    u8 shade);

/* @source 0x801D1118
 * @behavior Entry 1 of this overlay handler run at 0x801D1B48 (0x801D10E8,
 * 0x801D1118, 0x801D11A4), which the dispatcher func_801D10A4 selects with the
 * unsigned work byte at offset +0x1 of the scratchpad work object published at
 * pointer slot 0x1F800044 and calls with no arguments: it takes that object
 * countdown byte at +0x9, scales it by 16, negates the product and passes the
 * low byte to the in-payload draw helper
 * drawFullScreenRedGradientBandCenteredOnScreenCenterScenarioSce15ef200_801D19A8, which submits two
 * 320-pixel-wide semi-transparent quads covering the screen and writes that
 * byte into the red component of every vertex, so the countdown drives a red
 * tint that deepens as the countdown expires. It then ticks the same countdown
 * byte down by one and, when the decremented byte reaches zero, re-arms it with
 * 0x10 and advances the work byte at +0x1 by one, so the next frame of that run
 * dispatches entry 2 (0x801D11A4). All three later scratchpad accesses are
 * separate loads of the 0x1F800044 cell because the byte store through the
 * loaded pointer may alias the cell. It takes no arguments, returns nothing and
 * its 0x18-byte frame only keeps $ra at 0x10 across the helper call.
 *
 * Evidence: the fourteen words of the run at 0x801D1B1C name 0x801D0C70,
 * 0x801D0CA0, 0x801D0D00, 0x801D0D5C, 0x801D0DE8, 0x801D0E30, 0x801D0EC4,
 * 0x801D0F68, 0x801D0F9C, 0x801D1008, 0x801D1050, 0x801D10E8, 0x801D1118 and
 * 0x801D11A4, and the neighbouring dispatcher records 0x801D1B48 as entry 11 of
 * that run, so this body is the second entry of the dispatched window and the
 * three words at 0x801D1B48 name its own run. The helper argument is read off
 * 0x801D19A8: it keeps $a0 in $s2, opens the 0x380 texture page draw mode, emits
 * two 0x140 by 0x78 POLY_G4 primitives through SetPolyG4 and SetSemiTrans and
 * stores $s2 into the red byte of each vertex (0x04, 0x0C, 0x14, 0x1C) with the
 * green and blue bytes cleared, so the argument is an unsigned shade byte.
 *
 * Shape evidence: the tail from the countdown reload through the return is
 * byte-identical to the exact span
 * tickCountdownReload16AndAdvanceWorkByte1ScenarioSce15ef200_801D0D00 in this
 * target, which records the same re-arm of 0x10, the same andi $v0,$v0,0xFF
 * that the promoted u8 decrement produces and the same re-read of the pointer
 * cell after every store; this body is that sequence behind a frame that keeps
 * $ra across the helper call. The overlay twin func_801D0DAC in
 * emi/scenario/sce15ef3/00 is the same 0x8C-byte statement sequence, so it is
 * shape evidence only: three of its bytes differ, the shift count (5 there
 * instead of 4), the jal displacement to its own helper 0x801D1BAC instead of
 * 0x801D19A8, and the re-armed countdown (8 there instead of 0x10).
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawRedTintFromNegatedCountdownThenTickCountdownReload16AndAdvanceWorkByte1ScenarioSce15ef200_801D1118(void) {
  drawFullScreenRedGradientBandCenteredOnScreenCenterScenarioSce15ef200_801D19A8(
      -(D_1F800044[9] << 4));
  if (--D_1F800044[9] == 0) {
    D_1F800044[9] = 0x10;
    D_1F800044[1]++;
  }
}

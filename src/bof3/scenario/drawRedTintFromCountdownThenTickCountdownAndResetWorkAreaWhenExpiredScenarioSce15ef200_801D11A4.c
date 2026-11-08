#include "bof3/bof3.h"

/* Published scratchpad work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. The
 * countdown store goes through a pointer loaded from that cell, so the call and
 * the later tick each reload the cell. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* In-payload draw helper shared with the sibling entry 0x801D1118: it submits
 * two 320-pixel-wide semi-transparent quads covering the screen and stores its
 * argument into the red component of every vertex. The parameter is a 32-bit
 * signed value, not a byte: this call site passes the already masked shade
 * through untouched - the original shows no truncation in the jal delay slot,
 * and declaring a byte here measured 28 of 29 instructions because that
 * truncation appeared - while the helper truncates on its own store. */
void drawFullScreenRedGradientBandCenteredOnScreenCenterScenarioSce15ef200_801D19A8(
    s32 shade);

/* Shared work-area reset helper of the game image: it zeroes work-object bytes
 * 0x00-0x04, including the work byte this overlay dispatches on, so the overlay
 * drops back to work state 0. No shipped header declares it, so the prototype
 * stays local, exactly as in the sibling lifts of the same helper. */
void func_80196070(void);

/* @source 0x801D11A4
 * @behavior Entry 1 of the overlay code-pointer table based at 0x801D1B48,
 * which the dispatcher 0x801D10A4 selects with the unsigned work byte at
 * offset +0x1 of the scratchpad work object published at pointer slot
 * 0x1F800044 and calls with no arguments: it reads that object countdown byte
 * at +0x9, keeps the full byte 0xFF as the shade while the countdown still
 * reads its re-arm value 0x10, and otherwise scales the countdown by 16 and
 * masks the product to its high nibble, so the tint dims as the countdown
 * expires. It passes that shade to the in-payload draw helper
 * drawFullScreenRedGradientBandCenteredOnScreenCenterScenarioSce15ef200_801D19A8,
 * which writes it into the red component of two full-screen quads, then ticks
 * the same countdown byte down by one and, when the decremented byte reaches
 * zero, resets the shared work area through func_80196070, which zeroes
 * work-object bytes 0x00-0x04 including both the countdown and that dispatch
 * byte, so the overlay drops back to work state 0 instead of re-arming. It
 * takes no arguments, returns nothing and its 0x18-byte frame only keeps $ra
 * at 0x10 across the two calls.
 *
 * Evidence: the fourteen words of the run at 0x801D1B1C name 0x801D0C70,
 * 0x801D0CA0, 0x801D0D00, 0x801D0D5C, 0x801D0DE8, 0x801D0E30, 0x801D0EC4,
 * 0x801D0F68, 0x801D0F9C, 0x801D1008, 0x801D1050, 0x801D10E8, 0x801D1118 and
 * 0x801D11A4; the dispatcher func_801D10A4 loads its call target as
 * %lo(D_801D1B48)($at) with $at advanced by the work byte scaled by four, so
 * this address is index 1 of that base, one word past the entry whose body
 * advances the same work byte by one. The shade role is read off the shared
 * helper's own span: it keeps $a0 in $s2 and stores that value to the red byte
 * of all four vertices of each quad with the green and blue bytes cleared.
 *
 * Shape evidence: the countdown reload, the decrement, the byte store, the
 * 0xFF mask and the zero guard down to the return are byte-identical to the
 * tail of the exact sibling 0x801D1118, which spells the same sequence as a
 * pre-decrement of the published cell; this body keeps that spelling and only
 * replaces that sibling's re-arm and work-byte advance with the reset call,
 * which leaves the guard branch delay slot a nop. The guard arm reads as the
 * branch target's own first instruction - the unscaled countdown shifted in
 * place - so the shift and the nibble mask are two assignments to the one local
 * rather than a single masked expression, which measured 27 of 29 with the
 * shift allocated to a fresh register; and the compare is an int compare, not a
 * narrow one, which rules out a byte-typed countdown local.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawRedTintFromCountdownThenTickCountdownAndResetWorkAreaWhenExpiredScenarioSce15ef200_801D11A4(
    void) {
  s32 shade;

  shade = D_1F800044[9];
  if (shade == 0x10) {
    shade = 0xFF;
  } else {
    shade <<= 4;
    shade &= 0xF0;
  }
  drawFullScreenRedGradientBandCenteredOnScreenCenterScenarioSce15ef200_801D19A8(
      shade);
  if (--D_1F800044[9] == 0) {
    func_80196070();
  }
}

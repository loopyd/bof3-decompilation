#include "bof3/ui/shop00_internal.h"

/* @source 0x801D5E88
 * @behavior shop sub-step handler at table entry 0x13 of the sub-step handler
 *           table D_801E530C, which func_801D46A4 dispatches with the UI
 *           sub-step byte D_80148652 as index: it switches on the overlay-local
 *           transition variant byte D_801E60EC, and every arm first emits the
 *           shop panel strip through this EMI's panel strip emitter
 *           func_801DAB90 with the fixed panel arguments (0x14, 0x12, 0x118,
 *           0x13) and the main-RAM CLUT-bank byte D_80144952 as the fifth
 *           argument. Variant 0x17 alone submits the texture row 0x1C at row
 *           0x16 through func_8014F800 with the value returned by
 *           func_801502D0(0x9C) as its fifth argument. The variants 0x14, 0x16
 *           and 0x18 and every value outside the unsigned six-value range
 *           0x14..0x19 submit that row with func_801502D0(D_801E60EC + 0x85)
 *           instead and then draw the panel row through the row emitter
 *           func_801D67B0 at the fixed coordinates (0x20, 0x30, 0). Variants
 *           0x15 and 0x19 submit the same variant-selected row and draw the
 *           sliding shop value panel through func_801DDFB0 at the origin
 *           (0x62, 0x4C) with the overlay flag byte D_801E60F8, the main-RAM
 *           byte D_8018E260 and the variant constant 1. Every arm then counts
 *           the frame timer phaseTimer down through a byte view of its address
 *           and, once the decremented byte reaches zero, restores the UI
 *           sub-step byte D_80148652 from the archived sub-step byte
 *           D_801E6094 the accepting handlers stored before arming this
 *           transition.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawShopVariantPanelThenRestoreUiSubstepOnTimerExpiry(void) {
  u8* timer;
  u8 next;

  /* The switch lowers to the unsigned range guard 0x14..0x19 plus the
   * six-entry jump table the original keeps in this overlay's leading data
   * block, which the manifest placement for this function supplies. */
  switch (D_801E60EC) {
  case 0x17:
    func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
    func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(0x9C));
    break;
  case 0x14:
  case 0x16:
  case 0x18:
  default:
    func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
    func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(D_801E60EC + 0x85));
    func_801D67B0(0x20, 0x30, 0);
    break;
  case 0x15:
  case 0x19:
    func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
    func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(D_801E60EC + 0x85));
    func_801DDFB0(0x62, 0x4C, D_801E60F8, D_8018E260, 1);
    break;
  }
  /* The original materializes the timer address once and reuses that base for
   * the countdown load and store (lui+addiu / lbu / sb through one register),
   * which the volatile symbol access folds and reloads instead; this function
   * takes the same explicitly non-volatile byte view of the shared cell that
   * the phase step func_801D408C uses. */
  timer = (u8*)&phaseTimer;
  next = *timer - 1;
  *timer = next;
  if (next == 0) {
    D_80148652 = D_801E6094;
  }
}

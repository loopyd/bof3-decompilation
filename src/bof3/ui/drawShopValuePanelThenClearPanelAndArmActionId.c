#include "bof3/ui/shop00_internal.h"

/* @source 0x801D5CB0
 * @behavior shop sub-step handler at table entry 0x11 of the sub-step handler
 *           table D_801E530C, which func_801D46A4 dispatches with the UI
 *           sub-step byte D_80148652 as index: it emits the shop panel strip
 *           through this EMI's panel strip emitter func_801DAB90 with the fixed
 *           panel arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM
 *           CLUT-bank byte D_80144952 as the fifth argument, submits the
 *           texture row 0x1C at row 0x16 through func_8014F800 with the value
 *           returned by func_801502D0(D_801E60EC + 0x85) as its fifth
 *           argument, draws the sliding shop value panel through func_801DDFB0
 *           at the origin (0x62, 0x4C) with the overlay flag byte D_801E60F8,
 *           the main-RAM byte D_8018E260 and the variant constant 1, and then
 *           clears the panel through func_801EEC30 with five zero arguments.
 *           When the selected panel action id D_8018E228 reads 0 or 2 it tests
 *           the main-exe gate func_801EF200: a nonzero result plays the main-exe
 *           sound cue 0x107 through the cue dispatcher func_8015DF18 and
 *           re-arms the action id with 7, while a zero result re-arms it with
 *           9. Every path then arms the UI sub-step byte D_80148652 with 0xC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawShopValuePanelThenClearPanelAndArmActionId(void) {
  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(D_801E60EC + 0x85));
  func_801DDFB0(0x62, 0x4C, D_801E60F8, D_8018E260, 1);
  func_801EEC30(0, 0, 0, 0, 0);
  if (D_8018E228 == 0 || D_8018E228 == 2) {
    if (func_801EF200() != 0) {
      func_8015DF18(0x107);
      D_8018E228 = 7;
    } else {
      D_8018E228 = 9;
    }
  }
  D_80148652 = 0xC;
}

#include "bof3/ui/shop00_internal.h"

/* @source 0x801D47AC
 * @behavior shop value-panel sub-step handler at table entry 2 of the sub-step
 *           handler table D_801E530C, which func_801D46A4 dispatches with the
 *           UI sub-step byte D_80148652 as index: it emits the shop panel strip
 *           through this EMI's panel strip emitter func_801DAB90 with the fixed
 *           panel arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM
 *           CLUT-bank byte D_80144952 as the fifth argument, submits the
 *           texture row 0x1C at row 0x16 through func_8014F800 with the value
 *           returned by func_801502D0(D_801E60EC + 0x85) as its fifth
 *           argument, and clears the panel through func_801EEC30 with five zero
 *           arguments twice, drawing the main-RAM value word D_8018E260 as 0
 *           before the first clear and as 1 before the second.
 *           The selected panel action id D_8018E228 is read before that second
 *           clear and again after it, and the two reads select which values the
 *           value panel offers: both reads apart from 0 and 2 play the main-exe
 *           sound cue 0x107 through the cue dispatcher func_8015DF18, clear the
 *           value-panel variant byte D_801E60F8 and arm the UI sub-step byte
 *           D_80148652 with 0xC instead. Every other combination arms the
 *           overlay-local pair of selectable values D_801E6088/D_801E608C with
 *           (0, 1) and the drawn value 0 for one read pair, with (0, 0) and the
 *           drawn value 0 for a first read of 0 or 2 followed by a second read
 *           outside that set, and with (1, 1) and the drawn value 1 for the
 *           reverse. Those paths arm the variant byte with 3, 1 and 2
 *           respectively and then re-arm the frame timer phaseTimer with 4 and
 *           advance the UI sub-step byte by one, which enters the value-panel
 *           slide-in step func_801D4970.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armShopValueSelectionThenAdvanceUiSubstep(void) {
  u32 armedActionId;
  u32 currentActionId;

  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(D_801E60EC + 0x85));
  D_8018E260_WORD = 0;
  func_801EEC30(0, 0, 0, 0, 0);
  armedActionId = D_8018E228;
  D_8018E260_WORD = 1;
  func_801EEC30(0, 0, 0, 0, 0);
  currentActionId = D_8018E228;
  if (armedActionId == 0 || armedActionId == 2) {
    if (currentActionId == 0 || currentActionId == 2) {
      D_801E608C = 1;
      D_8018E260_WORD = 0;
      D_801E6088 = 0;
      D_801E60F8 = 3;
    } else {
      D_8018E260_WORD = 0;
      D_801E6088 = 0;
      D_801E608C = 0;
      D_801E60F8 = 1;
    }
  } else {
    if (currentActionId == 0 || currentActionId == 2) {
      D_801E6088 = 1;
      D_801E608C = 1;
      D_8018E260_WORD = 1;
      D_801E60F8 = 2;
    } else {
      func_8015DF18(0x107);
      D_80148652 = 0xC;
      D_801E60F8 = 0;
      return;
    }
  }
  phaseTimer = 4;
  D_80148652 = D_80148652 + 1;
}

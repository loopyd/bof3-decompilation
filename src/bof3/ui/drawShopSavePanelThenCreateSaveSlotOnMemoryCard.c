#include "bof3/ui/shop00_internal.h"

/* @source 0x801D5894
 * @behavior shop sub-step handler at table entry 0x0E of the sub-step handler
 *           table D_801E530C (the payload word at 0x801E5344 names this
 *           function), which func_801D46A4 dispatches with the UI sub-step byte
 *           D_80148652 as index: it publishes 3 into the panel-record count
 *           D_801E6078, emits the shop panel strip through this EMI's panel
 *           strip emitter func_801DAB90 with the fixed panel arguments
 *           (0x14, 0x12, 0x118, 0x13) and the main-RAM CLUT-bank byte
 *           D_80144952 as the fifth argument, submits the texture row 0x1C at
 *           row 0x16 through func_8014F800 with the value returned by
 *           func_801502D0(D_801E60EC + 0x85) as its fifth argument, draws the
 *           panel row through func_801D67B0(0x20, 0x30, 0) and clears the
 *           panel through func_801EEC30 with five zero arguments.
 *           While the main-RAM panel action id D_8018E228 reads nonzero it arms
 *           the sub-step byte D_80148652 with 0xC alone. Otherwise it formats
 *           the save name into the shared text buffer D_80145AD4 through
 *           sprintf with the payload save-name prefix D_801D0C20 and the
 *           selected slot byte D_801E6074, overwrites that buffer with the
 *           game-time stamp formatted from the payload string D_801D0C68 and
 *           the front-end clock bytes D_80144FC0, D_80144FC1 and D_80144FC2,
 *           and copies the stamp and then the name into the two fields of its
 *           own local record, which it submits through func_801EEC30 with the
 *           constants 1, 0x200, 0 and 0.
 *           When that submission does not report -1 it publishes the one-based
 *           slot byte D_801E6074 plus one into the bracket-entry byte
 *           D_801E6108, prepares the save-data image through
 *           prepareSaveDataImageAndPublishPanelRecord and, while the
 *           remaining-capacity byte D_801E60FC is nonzero, consumes one block
 *           of it and advances the kept-entry count D_801E610C - the counter
 *           pair that records the new directory entry the write creates. It
 *           then submits the same record through func_801EEC30 with the
 *           constants 2, 0x12B0 and 0 and, when that second submission also
 *           does not report -1, arms the panel action id with 0xA, the sub-step
 *           byte with 0xC, the frame timer phaseTimer with 0x3C and the
 *           overlay byte D_801E60F0 with 2.
 *           A -1 from either submission plays the cue 0x107 through
 *           func_8015DF18 after rewriting an action id of 3 with 6, then arms
 *           the sub-step byte with 0xC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawShopSavePanelThenCreateSaveSlotOnMemoryCard(void) {
  char name[0x18];
  char stamp[0x40];

  D_801E6078 = 3;
  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(D_801E60EC + 0x85));
  func_801D67B0(0x20, 0x30, 0);
  func_801EEC30(0, 0, 0, 0, 0);
  if (D_8018E228 != 0) {
    D_80148652 = 0xC;
    return;
  }
  sprintf((char*)D_80145AD4, D_801D0C20, D_801E6074);
  sprintf((char*)D_80145AD4, D_801D0C68, D_80144FC0, D_80144FC1, D_80144FC2);
  strcpy(stamp, (char*)D_80145AD4);
  sprintf((char*)D_80145AD4, D_801D0C20, D_801E6074);
  strcpy(name, (char*)D_80145AD4);
  if ((s8)func_801EEC30(1, name, 0x200, 0, 0) != -1) {
    D_801E6108 = D_801E6074 + 1;
    prepareSaveDataImageAndPublishPanelRecord();
    if (D_801E60FC != 0) {
      D_801E60FC = D_801E60FC - 1;
      D_801E610C = D_801E610C + 1;
    }
    if ((s8)func_801EEC30(1, name, 2, 0x12B0, 0) != -1) {
      D_8018E228 = 0xA;
      D_80148652 = 0xC;
      phaseTimer = 0x3C;
      D_801E60F0 = 2;
      return;
    }
  }
  if (D_8018E228 == 3) {
    D_8018E228 = 6;
  }
  func_8015DF18(0x107);
  D_80148652 = 0xC;
}

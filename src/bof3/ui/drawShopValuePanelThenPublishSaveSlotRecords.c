#include "bof3/ui/shop00_internal.h"

/* @source 0x801D4BBC
 * @behavior shop sub-step handler at table entry 5 of the sub-step handler
 *           table D_801E530C, which func_801D46A4 dispatches with the UI
 *           sub-step byte D_80148652 as index: emits the shop panel strip
 *           through this EMI's panel strip emitter func_801DAB90 with the fixed
 *           panel arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM
 *           CLUT-bank byte D_80144952 as the fifth argument, submits the
 *           texture row 0x1C at row 0x16 through func_8014F800 with the value
 *           returned by func_801502D0(D_801E60EC + 0x85) as its fifth
 *           argument, draws the sliding shop value panel through
 *           func_801DDFB0 at the origin (0x62, 0x4C) with the overlay flag byte
 *           D_801E60F8, the main-RAM byte D_8018E260 and the variant constant
 *           1, then clears the panel through func_801EEC30 with five zero
 *           arguments.
 *           When the selected action id D_8018E228 is zero it collects the
 *           save-slot list through collectSaveSlotsThenCountFreeBlocks: while
 *           the collected entry count D_801E6078 is nonzero it rebuilds the
 *           three panel records through rebuildPanelRecordTable, arms that
 *           count with 3, clears the selected row byte D_801E6074 and advances
 *           the UI sub-step byte by one; otherwise it selects the action id 5
 *           when the remaining-capacity byte D_801E60FC is nonzero and 4 when
 *           that byte is zero. That empty-list path and the nonzero action id
 *           both play the main-exe sound cue 0x107 through the cue dispatcher
 *           func_8015DF18 and arm the UI sub-step byte with 0xC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawShopValuePanelThenPublishSaveSlotRecords(void) {
  u8* step;
  u8 next;

  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(D_801E60EC + 0x85));
  func_801DDFB0(0x62, 0x4C, D_801E60F8, D_8018E260, 1);
  func_801EEC30(0, 0, 0, 0, 0);
  if (D_8018E228 == 0) {
    collectSaveSlotsThenCountFreeBlocks();
    if (D_801E6078 != 0) {
      rebuildPanelRecordTable();
      /* The original materialises the sub-step byte's address once, loads the
       * byte before the two arming stores and keeps the incremented value for
       * the store that fills the return jump's delay slot, so the byte is read
       * into a local at the original's load position. */
      step = &D_80148652;
      next = *step;
      D_801E6078 = 3;
      D_801E6074 = 0;
      *step = next + 1;
      return;
    }
    if (D_801E60FC != 0) {
      D_8018E228 = 5;
    } else {
      D_8018E228 = 4;
    }
  }
  func_8015DF18(0x107);
  D_80148652 = 0xC;
}

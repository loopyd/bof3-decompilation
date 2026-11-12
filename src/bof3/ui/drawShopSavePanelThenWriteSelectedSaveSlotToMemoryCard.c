#include "bof3/ui/shop00_internal.h"

/* @source 0x801D51B8
 * @behavior shop sub-step handler at table entry 0xA of the sub-step handler
 *           table D_801E530C, which func_801D46A4 dispatches with the UI
 *           sub-step byte D_80148652 as index: it emits the shop panel strip
 *           through this EMI's panel strip emitter func_801DAB90 with the fixed
 *           panel arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM
 *           CLUT-bank byte D_80144952 as the fifth argument, submits the
 *           texture row 0x1C at row 0x16 through func_8014F800 with the value
 *           returned by func_801502D0(D_801E60EC + 0x85) as its fifth
 *           argument, draws the panel row through func_801D67B0(0x20, 0x30, 0)
 *           and clears the panel through func_801EEC30 with five zero
 *           arguments.
 *           It writes the selected save slot only while the main-RAM panel
 *           action id D_8018E228 still reads zero. That arm formats the save
 *           name into the shared text buffer D_80145AD4 through sprintf with
 *           the payload's 16-character save-name prefix D_801D0C20, whose
 *           format text continues into the payload's "%02d", and the selected
 *           slot byte D_801E6074; it then hands that name to func_801EEC30 with
 *           the constants 1, 1 and 0 and plays the cue 0x107 through
 *           func_8015DF18 when the sign-extended low byte of the result reads
 *           -1. Otherwise it re-formats the name, publishes the units digit
 *           byte D_80145AE5 of that name plus 0xD1 into the bracket-entry byte
 *           D_801E6108, prepares the save-data image through
 *           prepareSaveDataImageAndPublishPanelRecord, re-formats the name once
 *           more and hands it to func_801EEC30 with the constants 2, 0x12B0 and
 *           0, playing the same 0x107 cue on the -1 result. When that second
 *           submission reports no failure it publishes the selected slot's
 *           panel record through func_801D6CE4 with the slot byte D_801E6074
 *           and the live save-data image base D_801448D8, arms the panel
 *           action id with 8, the sub-step byte and the frame timer phaseTimer
 *           with 0xC and 0x3C, and the overlay byte D_801E60F0 with 2. Every
 *           exit arms the sub-step byte with 0xC, which enters the following
 *           table entry func_801D543C; an action id other than zero arms that
 *           byte alone.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The original holds the CLUT-bank cell address in one callee-saved register
 * for the whole body and derives the save-data image base from it (0x80144952
 * - 0x7A = 0x801448D8) instead of materializing that second address, which is
 * why the frame keeps five words and the image argument costs one `addiu` off
 * the register. Writing the image argument as the plain `D_801448D8` extern
 * instead measured 97/118 instructions over 468 bytes: the compiler dropped
 * that register, shrank the frame from 0x30 to 0x28 and moved every stack
 * offset in the body. The two `-1` failure arms share one block because the
 * source tests the success arm twice (`!= -1`) and leaves the cue and the 0xC
 * arm at the end of the body, where the original also places them. */
void drawShopSavePanelThenWriteSelectedSaveSlotToMemoryCard(void) {
  u8* clutCell;
  clutCell = &D_80144952;

  func_801DAB90(0x14, 0x12, 0x118, 0x13, *clutCell);
  func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(D_801E60EC + 0x85));
  func_801D67B0(0x20, 0x30, 0);
  func_801EEC30(0, 0, 0, 0, 0);
  if (D_8018E228 != 0) {
    D_80148652 = 0xC;
    return;
  }
  sprintf((char*)D_80145AD4, D_801D0C20, D_801E6074);
  if ((s8)func_801EEC30(1, D_80145AD4, 1, 1, 0) != -1) {
    sprintf((char*)D_80145AD4, D_801D0C20, D_801E6074);
    D_801E6108 = D_80145AE5 + 0xD1;
    prepareSaveDataImageAndPublishPanelRecord();
    sprintf((char*)D_80145AD4, D_801D0C20, D_801E6074);
    if ((s8)func_801EEC30(1, D_80145AD4, 2, 0x12B0, 0) != -1) {
      func_801D6CE4(D_801E6074, clutCell - 0x7A);
      D_8018E228 = 8;
      D_80148652 = 0xC;
      phaseTimer = 0x3C;
      D_801E60F0 = 2;
      return;
    }
  }
  func_8015DF18(0x107);
  D_80148652 = 0xC;
}

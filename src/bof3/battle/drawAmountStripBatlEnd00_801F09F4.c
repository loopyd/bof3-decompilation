#include "bof3/bof3.h"

extern s32 D_80144F50;
extern u8 D_80149118[];
extern u8 D_801EEC4C[];
extern u8 D_801F0CD0[];

extern int sprintf(char* buffer, const char* format, ...);

/* The narrow battle message-panel strip drawer of the main executable at
 * 0x801D8270: opens the panel draw mode and outlines the rectangle inset to
 * (arg0 + 2, arg1 + 2) through (arg0 + 0x65, arg1 + 0x10) from the panel tile
 * and quadrangle rows (lifted exact at this address as
 * drawBattlePanelMessageStrip in the battle/03 target). Still an
 * address-anchored EXE span here, so it is declared at its own address. */
extern void func_801D8270(s16 arg0, s16 arg1);

/* Shared text drawers of the main executable. func_8014FF0C submits one
 * NUL-terminated character run at (arg0, arg1); func_8014F800 submits a run at
 * (arg0, arg1) with arg2 and arg3 as given and the run pointer on the stack as
 * its fifth argument. Declared here for their addresses. */
extern void func_8014FF0C(s16 arg0, s16 arg1, s32 arg2, const void* arg3);
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* @source 0x801F09F4
 * @behavior Battle-end amount strip: draws the narrow message-panel strip at
 *   (0x6E, 0x28) through the EXE drawer func_801D8270, formats the main-RAM
 *   word 0x80144F50 through the overlay format 0x801EEC4C ("%7d") into the
 *   main-RAM 0x80149118 text buffer, submits that buffer at (0x7E, 0x2A)
 *   through func_8014FF0C, then submits the overlay one-character run at
 *   0x801F0CD0 at (0xB6, 0x2A) with a zero third argument and 0xFF as the
 *   fourth through func_8014F800. That run starts seven 8-pixel digits after
 *   the buffer anchor, so it follows the formatted amount. Takes no arguments
 *   and returns nothing; the 0x20-byte frame keeps $ra at 0x1C($sp), $s0 at
 *   0x18($sp) across the four calls and the fifth argument of the last call at
 *   0x10($sp).
 * @status exact
 * @match 100.00
 * @residual none
 * 0x801F09F4 is entry 1 (the drawn state) of the two-word local state table
 * built by dispatchPanelStateLocalTableBatlEnd00_801F097C, whose entry 0
 * advanceStateByte3BatlEnd00_801F09D4 advances the shared panel state byte
 * +0x03 from 2 to 3; that dispatcher is word 5 of the panel handler run at
 * 0x801EEC28 and its sibling at 0x801F0768 draws the per-battler experience
 * panel in the state our strip follows. The formatted word 0x80144F50 is the
 * party money cell, the value the shop purchase step subtracts from and the
 * shop value panels print; the one-character run at 0x801F0CD0 is the unit
 * glyph drawn directly after those seven digits.
 */
void drawAmountStripBatlEnd00_801F09F4(void) {
  char* buffer;

  func_801D8270(0x6e, 0x28);
  buffer = (char*)D_80149118;
  sprintf(buffer, (const char*)D_801EEC4C, D_80144F50);
  func_8014FF0C(0x7e, 0x2a, 0, buffer);
  func_8014F800(0xb6, 0x2a, 0, 0xff, (u32)D_801F0CD0);
}

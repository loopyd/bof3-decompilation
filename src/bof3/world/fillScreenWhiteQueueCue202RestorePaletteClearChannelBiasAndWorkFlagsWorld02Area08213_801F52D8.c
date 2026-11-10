#include "bof3/bof3.h"

extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */
extern u8 D_8014933A; /* @source 0x8014933A @kind data */
extern s8 D_80145EEF; /* @source 0x80145EEF @kind data */
extern s8 D_80145EEE; /* @source 0x80145EEE @kind data */
extern s8 D_80145EED; /* @source 0x80145EED @kind data */

extern void func_8017A9A4(u32 primitive); /* @source 0x8017A9A4 */
extern void func_8014E5A0(u32 order, u32 size); /* @source 0x8014E5A0 */
extern void func_8015DF18(u16 cue); /* @source 0x8015DF18 */
extern void func_80196070(void); /* @source 0x80196070 */
extern void scalePaletteChannelsWorld02Area08213_801F5388(s32 scale);

/* @source 0x801F52D8
 * @behavior Final handler of the step table at 0x801F57D0. Initializes a
 *           flat quad at the shared primitive cursor, sets its corners to
 *           (0,0), (320,0), (0,240), (320,240) and its RGB bytes to 255,
 *           then appends it with ordering class 4 and size 0x18. Queues cue
 *           0x202, sets byte 0x8014933A to 1 and restores the palette through
 *           the local scale helper with unity scale 0x80. Clears the shared
 *           channel-bias bytes 0x80145EEF, 0x80145EEE and 0x80145EED in that
 *           order, then calls 0x80196070 to clear scratch work-header flags.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void fillScreenWhiteQueueCue202RestorePaletteClearChannelBiasAndWorkFlagsWorld02Area08213_801F52D8(
    void) {
  u32 prim;

  prim = (u32)g_PrimCursor;
  func_8017A9A4(prim);
  *(s16*)(prim + 8) = 0;
  *(s16*)(prim + 10) = 0;
  *(s16*)(prim + 12) = 320;
  *(s16*)(prim + 14) = 0;
  *(s16*)(prim + 16) = 0;
  *(s16*)(prim + 18) = 240;
  *(s16*)(prim + 20) = 320;
  *(s16*)(prim + 22) = 240;
  *(u8*)(prim + 4) = 255;
  *(u8*)(prim + 5) = 255;
  *(u8*)(prim + 6) = 255;
  func_8014E5A0(4, 0x18);
  func_8015DF18(0x202);
  D_8014933A = 1;
  scalePaletteChannelsWorld02Area08213_801F5388(0x80);
  D_80145EEF = 0;
  D_80145EEE = 0;
  D_80145EED = 0;
  func_80196070();
}

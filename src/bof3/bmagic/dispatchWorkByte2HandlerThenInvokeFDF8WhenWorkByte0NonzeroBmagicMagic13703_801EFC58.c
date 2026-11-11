#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The compiled original re-materializes the cell address for every
 * access, so the cell is reached through this target-map symbol rather than
 * through a constant address. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Work byte 2 step-handler run this overlay dispatches through, indexed at its
 * own base 0x801F0114: entry 0 = 0x801EFCC0, entry 1 = 0x801EFD00,
 * entry 2 = 0x801EFD50, entry 3 = 0x801EFD9C, the four step handlers that
 * follow this span. The base is the tail of the overlay code pointer run at
 * 0x801F00F8 and no map row owns it, so the reference is bound by the target's
 * generated undefined-symbol table. */
extern void (*D_801F0114[])(void); /* @source 0x801F0114 @kind data */

/* Overlay quad-drawing routine: sets the draw mode, then a poly G4 and semi
 * transparency on the primitive cursor and fills one 0x13F by 0xEF sprite
 * quad. */
extern void func_801EFDF8(void); /* @source 0x801EFDF8 @kind function */

/* @source 0x801EFC58
 * @behavior Dispatches one frame of the overlay work object through its work
 * byte 2 step table: the work-object pointer cell at 0x1F800044 is read before
 * the frame adjustment and its unsigned byte at offset 2, scaled by four,
 * selects the code pointer at 0x801F0114 plus that offset, which is invoked
 * with no arguments. The table is indexed where it lives, so no table word is
 * copied into the frame and the selector is not bounds-checked. After the
 * handler returns the pointer cell is read a second time and, when the work
 * object's byte 0 is non-zero, the overlay quad-drawing routine at 0x801EFDF8
 * runs. It takes no arguments and returns nothing; its 0x18-byte frame keeps
 * only $ra at 0x10, and it reads and writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeFDF8WhenWorkByte0NonzeroBmagicMagic13703_801EFC58(
    void) {
  u8 *work = g_battle_work;

  D_801F0114[work[2]]();
  work = g_battle_work;
  if (work[0] != 0) {
    func_801EFDF8();
  }
}

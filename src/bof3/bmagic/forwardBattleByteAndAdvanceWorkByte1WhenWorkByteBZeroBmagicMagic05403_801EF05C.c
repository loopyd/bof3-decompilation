#include "bof3/bof3.h"

/* Shared helper in the main executable, outside this overlay's image; bound
 * through this target's generated undefined_funcs_auto.txt. The exact bmagic
 * siblings pass it the same battle target byte, so its role beyond receiving
 * that byte remains unestablished. */
extern void func_800A9820(u8 value); /* @source 0x800A9820 */

/* Scratchpad work-object pointer cell. Declared as a named symbol rather than
 * through SPAD_PTR_SLOT so that the second access re-materialises the cell
 * address with its own lui after the helper call, which is what the original
 * does: the call may store through the loaded pointer and alias the cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Battle target selector byte in main RAM, outside this overlay's image; bound
 * through this target's generated undefined_syms_auto.txt. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* @source 0x801EF05C
 * @behavior Entry 1 of the four-entry code pointer table at 0x801EEC04 that
 *           the overlay dispatcher at 0x801EEC2C indexes with work byte 1.
 *           Reads the work-object pointer cell at 0x1F800044 and tests work
 *           byte 0xB. Only when that byte is zero does it forward the unsigned
 *           battle target byte 0x80146384 to helper 0x800A9820, re-read the
 *           pointer cell, and increment work byte 1 at byte width; otherwise
 *           it returns without touching anything. The 0x18-byte frame only
 *           keeps $ra across the call. It takes no arguments, returns nothing
 *           and writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardBattleByteAndAdvanceWorkByte1WhenWorkByteBZeroBmagicMagic05403_801EF05C(void) {
  if (D_1F800044[0xB] == 0) {
    func_800A9820(D_80146384);
    D_1F800044[1]++;
  }
}

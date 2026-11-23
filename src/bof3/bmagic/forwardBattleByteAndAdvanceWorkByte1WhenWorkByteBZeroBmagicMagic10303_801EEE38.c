#include "bof3/bof3.h"

/* Shared helper in the main executable, outside this overlay's image; bound
 * through this target's generated undefined_funcs_auto.txt. The exact bmagic
 * siblings pass it the same battle target byte, so its role beyond receiving
 * that byte remains unestablished. */
extern void func_800A9820(u8 value); /* @source 0x800A9820 @kind function */

/* Scratchpad work-object pointer cell, bound by this target's own map row.
 * Declared as a named symbol rather than through SPAD_PTR_SLOT so that the
 * second access re-materialises the cell address with its own lui after the
 * helper call, which is what the original does: the call may store through the
 * loaded pointer and alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Battle target selector byte in main RAM, outside this overlay's image; bound
 * through this target's generated undefined_syms_auto.txt, where the remaining
 * asm span func_801EEFF4 still references it. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* @source 0x801EEE38
 * @behavior Entry 1 of the three-entry code pointer run in this payload's
 *           leading data at 0x801EEC04 (0x801EEC70, 0x801EEE38, 0x801EEE94).
 *           The run's second word at 0x801EEC08 holds this address, and its
 *           consumer is the payload's own dispatcher
 *           callWorkHandlerTableec04, which
 *           copies the three words into its 0x28-byte frame and calls the entry
 *           named by the scratchpad work object's unsigned byte 0x1. This body
 *           reads the work-object pointer cell at 0x1F800044 and tests work
 *           byte 0xB. Only when that byte is zero does it forward the unsigned
 *           battle target byte 0x80146384 to helper 0x800A9820, re-read the
 *           pointer cell, and increment work byte 1 at byte width; otherwise it
 *           returns without touching anything. The 0x18-byte frame only keeps
 *           $ra at 0x10 across the call. No jal and no other aligned pointer
 *           word in the payload names 0x801EEE38, so the dispatcher reached
 *           through the run is its only caller. It takes no arguments, returns
 *           nothing and writes no other state. A payload compare of this
 *           0x5C-byte span against 0x801EF05C of emi/bmagic/magic054/03 came
 *           out equal in all 92 bytes, and that sibling had measured exact, so
 *           this body transcribes the sibling
 *           forwardBattleByteAndAdvanceWorkByte1WhenWorkByteBZeroBmagicMagic05403_801EF05C
 *           unchanged, binding the work cell to this target's own map row
 *           g_battle_work and keeping this payload's own helper and data
 *           addresses.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardBattleByteAndAdvanceWorkByte1WhenWorkByteBZeroBmagicMagic10303_801EEE38(void) {
  if (g_battle_work[0xB] == 0) {
    func_800A9820(D_80146384);
    g_battle_work[1]++;
  }
}

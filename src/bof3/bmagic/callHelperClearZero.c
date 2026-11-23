#include "bof3/bof3.h"

/* Shared battle event helper at 0x800AEF98, outside this overlay's image and
 * owned by the battle targets. This body only calls it, so the prototype
 * follows the call site: a byte-sized event selector as the first argument and
 * a byte-sized value as the second, with no result used. This target's
 * generated undefined_funcs_auto.txt already binds the address, so the name
 * needs no target-map row. */
extern void func_800AEF98(s32 event, s32 value); /* @source 0x800AEF98 @kind function */

/* Scratchpad work-object pointer cell bound at 0x1F800044 by this target's map.
 * The original re-materializes the cell address for the work byte B read before
 * the frame is created (lui $v0,0x1F80 / lw $v0,68($v0) then addiu $sp,-0x18 /
 * sw $ra,0x10($sp) / lbu 0xB($v0)) and again for the work byte 1 increment
 * after the call. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map already binds the address through
 * the row D_801EB4E0 = 0x801EB4E0, so the name needs no target-map row, and the
 * record is touched one byte at a time. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EEF38
 * @behavior Work byte 1 state 1 of this overlay: entry 1 of the three-word
 *           code-pointer run at 0x801EEC04 (0x801EED30, 0x801EEF38,
 *           0x801EEFA8), which the dispatcher at 0x801EEC28 copies into its
 *           frame and indexes with the scratchpad work object's byte 1, so this
 *           body runs while work byte 1 is 1. It reads the work-object pointer
 *           cell at 0x1F800044 and, only when the work object's byte at offset
 *           0xB is zero, calls the shared battle helper at 0x800AEF98 with the
 *           event selector 4 and the value 0, then clears bit 0x40 of byte 0 of
 *           the shared battle work record reached through the pointer cell at
 *           0x801EB4E0 with the 0xBF mask, and finally re-reads the scratchpad
 *           cell and increments work byte 1 at byte width, which selects run
 *           entry 2 (0x801EEFA8). The 4 is materialized in the guard branch's
 *           delay slot and the second argument is zeroed in the call's delay
 *           slot, so the record load follows the call. Takes no arguments,
 *           returns nothing, and the 0x18-byte frame only keeps $ra at 0x10
 *           across that call. The roles of work byte B, work byte 1 beyond that
 *           dispatch use and record byte 0 bit 0x40 are not established here, so
 *           they stay offset-anchored.
 *
 *           The 112-byte span is byte-identical to the exact lift at 0x801EEF68
 *           of bmagic/magic017/03 (one payload cmp, sha1 0e54b2e8), so the
 *           statement order, operand forms and declaration style are
 *           transcribed from it while both cell bindings stay this target's own
 *           map names. The same-address walk at 0x801EF1FC of bmagic/magic003/03
 *           differs from this span in its block order, computing the flag clear
 *           ahead of the call and filling the call's delay slot with the store,
 *           so it is shape evidence and not a clone source.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callHelperClearZero(
    void) {
  if (g_battle_work[0xB] == 0) {
    func_800AEF98(4, 0);
    D_801EB4E0[0] &= 0xBF;
    g_battle_work[1]++;
  }
}

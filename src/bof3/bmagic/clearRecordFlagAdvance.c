#include "bof3/bof3.h"

/* Shared battle buffer-copy helper at 0x800AFC54, outside this overlay's image
 * and owned by the battle targets (raw entry, emi/battle/battle/15@0x800AFC54).
 * This lift only calls it and sets no argument register, so the prototype
 * follows the call site and returns nothing. Splat binds the address through
 * this target's generated undefined_funcs_auto.txt, so no target-map row is
 * added here. */
extern void func_800AFC54(void); /* @source 0x800AFC54 @kind function */

/* Scratchpad work-object pointer cell shared by this overlay's handlers. The
 * original re-materializes the cell address (lui $vN,0x1F80 / lw $vN,68($vN))
 * for both reads in this function, so the cell stays an ordinary pointer and
 * the accesses spell the fixed address; the scratchpad needs no map row. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is added here; byte 0 of that
 * record carries the 0x40 flag this body clears. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */

/* @source 0x801EEF34
 * @behavior Work byte 1 state 1 of this overlay: entry 1 of the three-entry
 *           code pointer run at 0x801EEC04 (0x801EECC4, 0x801EEF34,
 *           0x801EEFA4), the word the run stores at 0x801EEC08 and the
 *           dispatcher at 0x801EEC64 selects on unsigned byte 1 of the work
 *           object held in the scratchpad pointer cell 0x1F800044. It reads
 *           that object's byte 0xB and, only when the byte is zero, calls the
 *           shared battle buffer-copy helper at 0x800AFC54 with no arguments,
 *           clears bit 0x40 of byte 0 of the shared battle work record read
 *           through the pointer at 0x801EB4E0 with the 0xBF mask, and then
 *           re-reads the scratchpad cell and increments work byte 1 with a
 *           byte-width add, which selects run entry 2 (0x801EEFA4). The record
 *           base stays in one register across its read, mask and store, the
 *           store group follows the call rather than filling its delay slot,
 *           the pointer-cell load precedes the frame, and the 0x18-byte frame
 *           only keeps $ra at 0x10 across the call. Nothing is returned and no
 *           other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordFlagAdvance(void) {
  if (D_1F800044[0xB] == 0) {
    func_800AFC54();
    D_801EB4E0[0] &= 0xBF;
    D_1F800044[1]++;
  }
}

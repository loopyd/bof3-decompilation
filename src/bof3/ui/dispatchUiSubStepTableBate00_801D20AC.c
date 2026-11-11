#include "bof3/bof3.h"

/* Shared UI sub-step byte of the main executable, the cell the BATE overlay's
 * other handlers advance by one as a sub-step completes. It lies outside this
 * EMI payload and is read at the original unsigned byte width.
 * @source 0x80148652 @kind data
 */
extern u8 D_80148652;

/* Two-entry sub-step handler pointer table of this overlay, holding the
 * addresses 0x801D20E8 and 0x801D211C. */
extern void (*D_801D8770[])(void); /* @source 0x801D8770 @kind table */

/* @source 0x801D20AC
 * @behavior Sub-step dispatcher of the BATE overlay: reads the shared UI
 *           sub-step byte D_80148652, scales it by four, loads the entry of
 *           this overlay's two-entry sub-step handler pointer table D_801D8770
 *           at that index and calls it, so the handler runs and control returns
 *           here. The 0x18-byte frame only saves the return address at 0x10
 *           across that call; the sub-step byte is not checked against the
 *           table size and nothing is returned. The index load's delay slot
 *           takes the frame setup and the slot after the table load stays a
 *           nop. Its only in-image reference is the word at 0x801D8764, entry 0
 *           of the three-entry phase handler table that
 *           dispatchUiPhaseTableBate00_801D2070 indexes, so it has no jal
 *           caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchUiSubStepTableBate00_801D20AC(void) {
  D_801D8770[D_80148652]();
}

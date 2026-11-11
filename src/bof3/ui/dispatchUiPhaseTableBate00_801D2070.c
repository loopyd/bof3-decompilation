#include "bof3/bof3.h"

extern u8 D_80148651; /* @source 0x80148651 @kind unknown */
/* Three-entry phase handler pointer table of this overlay, holding the
 * addresses 0x801D20AC, 0x801D2190 and 0x801D2298. */
extern void (*D_801D8764[])(void); /* @source 0x801D8764 @kind table */

/* @source 0x801D2070
 * @behavior phase-table dispatcher of the BATE overlay: reads the shared UI
 *           phase byte D_80148651, scales it by four, loads the entry of the
 *           three-entry phase handler pointer table D_801D8764 at that index
 *           and calls it, so the handler runs and control returns here. The
 *           0x18-byte frame only saves the return address at 0x10 across that
 *           call; the phase byte is not checked against the table size and
 *           nothing is returned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchUiPhaseTableBate00_801D2070(void) {
  D_801D8764[D_80148651]();
}

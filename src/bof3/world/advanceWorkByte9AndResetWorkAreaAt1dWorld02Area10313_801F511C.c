#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Naming the cell keeps the original's
 * per-access re-materialization (lui $vN,0x1F80 / lw $vN,68($vN)); the bare
 * constant address makes the compiler CSE 0x1F800044 into one saved register
 * and load through offset zero instead.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

void func_80196070(void);

/* @source 0x801F511C
 * @behavior Overlay step handler, entry 3 of the work-byte-2 handler table at
 *           0x801F5A28. It reads the work record published through the
 *           scratchpad pointer cell 0x1F800044, increments that record's byte
 *           at 0x09 - the drawing step counter the three earlier handlers of
 *           the same table ramp - and, when a fresh cell read shows the byte
 *           has reached 0x1D, calls the shared work-area reset helper
 *           func_80196070, which clears the scratchpad work-area header
 *           fields. Takes no arguments, returns nothing and uses a 0x18-byte
 *           frame for the saved return address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte9AndResetWorkAreaAt1dWorld02Area10313_801F511C(void) {
  u8* incCell;
  u8* stepCell;

  incCell = D_1F800044;
  incCell[9]++;
  stepCell = D_1F800044;
  if (stepCell[9] >= 0x1D) {
    func_80196070();
  }
}

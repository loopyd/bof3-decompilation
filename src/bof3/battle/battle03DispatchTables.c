#include "bof3/battle/battle03_internal.h"

/* @behavior dispatches the current local byte-2 state through its handler table.
 * @source 0x801DFA1C
 * @status exact
 * @match 100.00
 * @residual none
 */
BATTLE_DISPATCH_WORK_SLOT_TWO(dispatchLocalByte2Table, D_801EB1B4);

/* @source 0x801DFBDC
 * @behavior dispatches byte 2 of the scratchpad-resident state object through
 * its handler table.
 * @status exact
 * @match 100.00
 * @residual none
 */
BATTLE_DISPATCH_WORK_SLOT_TWO(dispatchScratchByte2Table, D_801EB1BC);

/* @behavior dispatches the current local state-2 class byte through its table.
 * @source 0x801E1670
 * @status exact
 * @match 100.00
 * @residual none
 */
BATTLE_DISPATCH_WORK_SLOT_TWO(dispatchState2ClassTable, D_801EB224);

/* @behavior dispatches the current local state-2 event byte through its table.
 * @source 0x801E1B64
 * @status exact
 * @match 100.00
 * @residual none
 */
BATTLE_DISPATCH_WORK_SLOT_TWO(dispatchState2EventTable, D_801EB26C);

/* @behavior dispatches the current local state-2 followup byte through its table.
 * @source 0x801E1CD8
 * @status exact
 * @match 100.00
 * @residual none
 */
BATTLE_DISPATCH_WORK_SLOT_TWO(dispatchState2FollowupTable, D_801EB274);

/* @behavior dispatches the current default-class local byte through its table.
 * @source 0x801E1E7C
 * @status exact
 * @match 100.00
 * @residual none
 */
BATTLE_DISPATCH_WORK_SLOT_TWO(dispatchDefaultClassTable, D_801EB27C);

/* @behavior dispatches the current local presentation state-1 byte through its
 * table.
 * @source 0x801E31C8
 * @status exact
 * @match 100.00
 * @residual none
 */
BATTLE_DISPATCH_WORK_SLOT_ONE(dispatchPresentationState1, D_801EB3B0);

/* @source 0x801E32AC
 * @behavior dispatches through D_801EB3E4 using byte 2 of the object addressed
 * by the non-volatile scratchpad pointer cell battleWork.
 * @status exact
 * @match 100.00
 * @residual none
 */
BATTLE_DISPATCH_WORK_SLOT_TWO(dispatchByte2TableB3e4, D_801EB3E4);

/* @source 0x801E3A00
 * @behavior dispatches through D_801EB404 using byte 2 of the object addressed
 * by the non-volatile scratchpad pointer cell battleWork.
 * @status exact
 * @match 100.00
 * @residual none
 */
BATTLE_DISPATCH_WORK_SLOT_TWO(dispatchByte2TableB404, D_801EB404);

/* @source 0x801E3B68
 * @behavior dispatches through D_801EB40C using byte two of the non-volatile
 * scratchpad pointer cell at 0x1F800044.
 * @status exact
 * @match 100.00
 * @residual none
 */
BATTLE_DISPATCH_WORK_SLOT_TWO(dispatchByte2TableB40c, D_801EB40C);

/* @source 0x801E4AE8
 * @behavior loads the non-volatile scratchpad pointer cell at 0x1F800044,
 * then invokes the byte-2-selected handler from the local 0x801EB460 table.
 * @status exact
 * @match 100.00
 * @residual none
 */
BATTLE_DISPATCH_WORK_SLOT_TWO(dispatchByte2TableB460, D_801EB460);

/* @source 0x801E4F64
 * @behavior reads the non-volatile scratchpad pointer cell at 0x1F800044,
 * selects its byte at offset 0x02, and calls that entry of D_801EB478.
 * @status exact
 * @match 100.00
 * @residual none
 */
BATTLE_DISPATCH_WORK_SLOT_TWO(dispatchByte2TableB478, D_801EB478);

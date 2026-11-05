#include "bof3/battle/battle15_internal.h"

/* @source 0x800AE4D0
 * @behavior Copies the four battle work-state handlers at 0x800969C4
 * (0x800AE574, 0x800AE640, 0x800AE6C8, 0x800AE728) and dispatches the entry
 * selected by scratchpad work byte 0x01; when work bytes 0x00 and 0x01 are
 * both non-zero it then invokes func_800AE9C4 with work byte 0x04.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchBattleWorkStateHandler(void)
{
    BattleWorkStateDispatchTable table;
    u8 *dispatch_work;
    u8 *gate_work;

    dispatch_work = g_battle_work;
    table = battleWorkStateHandlerTable;
    table.handlers[dispatch_work[1]]();
    gate_work = g_battle_work;
    if (gate_work[0] != 0 && gate_work[1] != 0) {
        func_800AE9C4(gate_work[4]);
    }
}

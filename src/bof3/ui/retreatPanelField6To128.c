#include "bof3/ui/shop00_internal.h"
#include "ui/panel_task.h"

/* @source 0x801E44A0
 * @behavior subtracts 0x10 from panel-task field 6; on underflow, clamps it to 0x80 and clears state.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
PANEL_RETREAT_FIELD6(retreatPanelField6To128, 0x10, 0x80)

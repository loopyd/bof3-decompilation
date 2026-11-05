#include "bof3/ui/shop00_internal.h"
#include "ui/panel_task.h"
#include "shared/ui/panel_task.inc"
#include "base/compiler.h"

/* @source 0x801E2D1C
 * @behavior subtracts 0x10 from the panel task field at offset 6, raises values below 0x3E, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_FIELD6(retreatPanelField6To62, 0x10, 0x3E)

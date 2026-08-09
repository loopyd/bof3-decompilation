#include "bof3/ui/shop00_internal.h"
#include "ui/panel_task.h"

/* @source 0x801E31C4
 * @behavior advances panel x toward 320 with the template clamp behavior.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
PANEL_ADVANCE_X(advancePanelXTo320, 320)

#include "bof3/ui/game00_internal.h"
#include "ui/panel_task.h"

/* @source 0x8019982C
 * @behavior advances panel x by 17 with the template clamp behavior.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
PANEL_ADVANCE_X(advancePanelXTo17, 17)

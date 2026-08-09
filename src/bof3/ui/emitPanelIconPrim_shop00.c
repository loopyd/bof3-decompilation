#include "bof3/ui/shop00_internal.h"
#include "ui/panel_task.h"

/* @source 0x801DA120
 * @behavior emits the panel icon primitive.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
PANEL_ICON_PRIM(emitPanelIconPrim)

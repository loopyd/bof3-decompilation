#include "bof3/ui/game00_internal.h"
#include "ui/panel_task.h"
#include "shared/ui/panel_task.inc"

/* @source 0x80199678
 * @behavior submits the current panel x and field-six coordinates to the panel helper.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_APPLY_POSITION(applyPanelPosition)

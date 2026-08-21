#include "bof3/ui/game00_internal.h"
#include "ui/panel_task.h"
#include "shared/ui/panel_task.inc"

/* @source 0x801995F8
 * @behavior retreats panel x with the template clamp behavior.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
PANEL_RETREAT_X(retreatPanelXToNeg170)

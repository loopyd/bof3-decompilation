#include "internal.h"
#include "ui/panel_task.h"

/* @source 0x801E2A30
 * @behavior retreats panel field six by 16 and clamps it to -20.
 */
PANEL_RETREAT_FIELD6(retreatPanelField6ToNeg20B, 0x10, -0x14)

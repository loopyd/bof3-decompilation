#include "bof3/battle/battle15_internal.h"
#include "ui/panel_task.h"
#include "shared/ui/panel_task.inc"

/* @source 0x800B23F8
 * @behavior Adds 32 to 16-bit panel x; signed results above 17 clamp to 17 and clear state.
 * @status exact
 * @match 100.00
 * @residual none
 */

PANEL_ADVANCE_X(advancePanelXTo17, 17)

/* @source 0x800B2218
 * @behavior Adds 32 to 16-bit panel x; signed results above 320 clamp to 320 and clear state.
 * @status exact
 * @match 100.00
 * @residual none
 */

PANEL_ADVANCE_X(advancePanelXTo320, 320)

/* @source 0x800B22AC
 * @behavior Adds 32 to 16-bit panel x; signed results above 320 clamp to 320 and clear state.
 * @status exact
 * @match 100.00
 * @residual none
 */

PANEL_ADVANCE_X(advancePanelXTo320B, 320)

/* @source 0x800B250C
 * @behavior Adds 32 to 16-bit panel x; signed results above 320 clamp to 320 and clear state.
 * @status exact
 * @match 100.00
 * @residual none
 */

PANEL_ADVANCE_X(advancePanelXTo320C, 320)

/* @source 0x800B20D4
 * @behavior UNKNOWN: exact behavior is not yet documented.
 * @status exact
 * @match 100.00
 * @residual none
 */

PANEL_APPLY_POSITION(applyPanelPosition)

/* @source 0x800B254C
 * @behavior UNKNOWN: exact behavior is not yet documented.
 * @status exact
 * @match 100.00
 * @residual none
 */

PANEL_RETREAT_X_CLAMP(retreatPanelXTo152, 0x98)

/* @source 0x800B23B8
 * @behavior UNKNOWN: exact behavior is not yet documented.
 * @status exact
 * @match 100.00
 * @residual none
 */

PANEL_RETREAT_X(retreatPanelXToNeg170, -0xAA)

#include "bof3/ui/shop00_internal.h"
#include "ui/panel_task.h"
#include "ui/panel_task_templates.h"
#include "base/compiler.h"

/* @source 0x801E2948
 * @behavior retreats panel field six by 16 and clamps it to -20.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_FIELD6(retreatPanelField6ToNeg20, 0x10, -0x14)

/* @source 0x801E2988
 * @behavior advances the panel task field_06 by 0x10, clamps to max 0x10, and
 *         clears state when the clamp is reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_FIELD6(advancePanelField6To16, 0x10)

/* @source 0x801E2A30
 * @behavior retreats panel field six by 16 and clamps it to -20.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_FIELD6(retreatPanelField6ToNeg20B, 0x10, -0x14)

/* @source 0x801E2A70
 * @behavior advances the panel task at offset 6 by 0x10, clamps to max 0x26, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_FIELD6(advancePanelField6To38, 0x26)

/* @source 0x801E2B14
 * @behavior subtracts 0x10 from the panel task field at offset 6, clamps to min -0x14, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_FIELD6_FLOOR(retreatPanelField6ToNeg20C, 0x10, -0x14)

/* @source 0x801E2B54
 * @behavior advances the panel task at offset 6 by 0x10, clamps to max 0x26, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_FIELD6_ONCE(advancePanelField6To38B, 0x26)

/* @source 0x801E2B94
 * @behavior passes the panel position and field six to the common UI helper.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_APPLY_POSITION(applyPanelPosition)

/* @source 0x801E2C9C
 * @behavior Decrements panel task x by 0x20, clamping at -0x96 and clearing
 *           state when the clamp fires.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X(retreatPanelXToNeg150, -0x96)

/* @source 0x801E2CDC
 * @behavior Adds 32 to 16-bit panel x; signed results above 17 clamp to 17 and clear state.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_X(advancePanelXTo17, 17)

/* @source 0x801E2D1C
 * @behavior subtracts 0x10 from the panel task field at offset 6, raises values below 0x3E, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_FIELD6(retreatPanelField6To62, 0x10, 0x3E)

/* @source 0x801E2E30
 * @behavior Decrements panel task x by 0x20, clamping at -0x96 and clearing
 *           state when the clamp fires.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X(retreatPanelXToNeg150B, -0x96)

/* @source 0x801E2E70
 * @behavior advances the panel task x position by 0x20, clamps to max 0xF, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_X(advancePanelXTo15, 0xF)

/* @source 0x801E31C4
 * @behavior Adds 32 to 16-bit panel x; signed results above 320 clamp to 320 and clear state.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_X(advancePanelXTo320, 320)

/* @source 0x801E3204
 * @behavior subtracts 0x20 from the panel task x position, clamps to max 0x84, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X_CLAMP(retreatPanelXTo132, 0x84)

/* @source 0x801E3244
 * @behavior subtracts 0x20 from the panel task x position, clamps to max 0x46, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X_CLAMP(retreatPanelXTo70, 0x46)

/* @source 0x801E3774
 * @behavior Adds 32 to 16-bit panel x; signed results above 320 clamp to 320 and clear state.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_X(advancePanelXTo320B, 320)

/* @source 0x801E37B4
 * @behavior subtracts 0x20 from the panel task x position, clamps to max 0x4B, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X_CLAMP(retreatPanelXTo75, 0x4B)

/* @source 0x801E3BF8
 * @behavior Adds 32 to 16-bit panel x; signed results above 320 clamp to 320 and clear state.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_X(advancePanelXTo320C, 320)

/* @source 0x801E3C38
 * @behavior subtracts 0x20 from the panel task x position, clamps to max 0x96, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X_CLAMP(retreatPanelXTo150, 0x96)

/* @source 0x801E3C78
 * @behavior advances the panel task x position by 0x20, clamps to max 0x28, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_X(advancePanelXTo40, 0x28)

/* @source 0x801E3CB8
 * @behavior retreats panel x with the template clamp behavior.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X(retreatPanelXToNeg170, -0xAA)

/* @source 0x801E3D4C
 * @behavior Adds 32 to 16-bit panel x; signed results above 320 clamp to 320 and clear state.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_X(advancePanelXTo320D, 320)

/* @source 0x801E3D8C
 * @behavior subtracts 0x20 from the panel task x position, clamps to max 0x96, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X_CLAMP(retreatPanelXTo150B, 0x96)

/* @source 0x801E3DCC
 * @behavior subtracts 0x20 from the panel task x position, clamps to max 0x11, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X_CLAMP(retreatPanelXTo17, 0x11)

/* @source 0x801E3E0C
 * @behavior advances the panel task x position by 0x20, clamps to max 0x96, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_X(advancePanelXTo150, 0x96)

/* @source 0x801E3EB4
 * @behavior Decrements panel task x by 0x20, clamping at -0x96 and clearing
 *           state when the clamp fires.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X(retreatPanelXToNeg150C, -0x96)

/* @source 0x801E3EF4
 * @behavior Adds 32 to 16-bit panel x; signed results above 17 clamp to 17 and clear state.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_X(advancePanelXTo17B, 17)

/* @source 0x801E4278
 * @behavior subtracts 0x20 from the panel task x position, clamps to min -120, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X(retreatPanelXToNeg120, -120)

/* @source 0x801E42B8
 * @behavior advances the panel task x position by 0x20, clamps to max 0x64, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_X(advancePanelXTo100, 0x64)

/* @source 0x801E42F8
 * @behavior subtracts 0x20 from the panel task x position, clamps to max 0x11, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X_CLAMP(retreatPanelXTo17B, 0x11)

/* @source 0x801E438C
 * @behavior Adds 32 to 16-bit panel x; signed results above 320 clamp to 320 and clear state.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_X(advancePanelXTo320E, 320)

/* @source 0x801E43CC
 * @behavior subtracts 0x20 from the panel task x position, clamps to max 0x8C, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X_CLAMP(retreatPanelXTo140, 0x8C)

/* @source 0x801E4460
 * @behavior advances the panel task field at offset 6 by 0x10, clamps to max 0xF0, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_FIELD6_ONCE(advancePanelField6To240, 0xF0)

/* @source 0x801E44A0
 * @behavior subtracts 0x10 from panel-task field 6; on underflow, clamps it to 0x80 and clears state.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_FIELD6(retreatPanelField6To128, 0x10, 0x80)

/* @source 0x801E4540
 * @behavior Adds 32 to 16-bit panel x; signed results above 320 clamp to 320 and clear state.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_X(advancePanelXTo320F, 320)

/* @source 0x801E4580
 * @behavior subtracts 0x20 from the panel task x position, clamps to max 0xD2, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X_CLAMP(retreatPanelXTo210, 0xD2)

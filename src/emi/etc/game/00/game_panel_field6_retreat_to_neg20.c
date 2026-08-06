#include "internal.h"
#include "ui/panel_task.h"

/* @source 0x80199938
 * @behavior retreats panel field six by 16 and clamps it to -20.
 */
PANEL_RETREAT_FIELD6(game_panel_field6_retreat_to_neg20, 0x10, -0x14)

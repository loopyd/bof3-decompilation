#include "internal.h"
#include "ui/panel_task.h"

/* @source 0x801E44A0
 * @behavior subtracts 0x10 from panel-task field 6; on underflow, clamps it to 0x80 and clears state.
 */
PANEL_RETREAT_FIELD6(shop_panel_field6_retreat_to_128, 0x10, 0x80)

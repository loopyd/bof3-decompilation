#include "internal.h"

/* @source 0x801E3CB8
 * @behavior decrements the panel task X coordinate by 32 pixels, clamps it to
 * -170, and clears the panel task state byte when the clamp is reached.
 */
#define BOF3_PANEL_TASK_FUNCTION func_801E3CB8
#include "shared/ui/retreat_panel_task_x.inc"

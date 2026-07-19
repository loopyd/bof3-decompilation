#include "internal.h"

/* @source 0x801997EC
 * @behavior decrements the panel task X coordinate by 32 pixels, clamps it to
 * -170, and clears the panel task state byte when the clamp is reached.
 */
#define BOF3_PANEL_TASK_FUNCTION func_801997EC
#include "shared/ui/retreat_panel_task_x.inc"

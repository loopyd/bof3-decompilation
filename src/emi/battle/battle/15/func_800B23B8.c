#include "internal.h"

/* @source 0x800B23B8
 * @behavior decrements the panel task X coordinate by 32 pixels, clamps it to
 * -170, and clears the panel task state byte when the clamp is reached.
 */
#define PANEL_TASK_FUNC func_800B23B8
#include "shared/ui/retreat_panel_task_x.inc"

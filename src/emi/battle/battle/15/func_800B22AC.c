#include "internal.h"

/* @source 0x800B22AC
 * @behavior advances the local panel task X position by 32 pixels, clamps it
 * to 320, and clears the preceding state byte when the clamp is reached.
 */
#define BOF3_PANEL_TASK_FUNCTION func_800B22AC
#define BOF3_PANEL_LIMIT 320
#include "shared/ui/advance_panel_task_x.inc"

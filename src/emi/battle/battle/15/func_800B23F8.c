#include "internal.h"

/* @source 0x800B23F8
 * @behavior advances the panel task X position by 32 pixels, clamps it to 17,
 * and clears the preceding state byte when the clamp is reached.
 */
#define PANEL_TASK_FUNC func_800B23F8
#define PANEL_LIMIT         17
#include "shared/ui/advance_panel_task_x.inc"

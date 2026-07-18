#include "internal.h"

/* @source 0x801E2CDC
 * @behavior advances the panel task X position by 32 pixels, clamps it to 17,
 * and clears the preceding state byte when the clamp is reached.
 */
#define BOF3_PANEL_TASK_FUNCTION func_801E2CDC
#define BOF3_PANEL_LIMIT 17
#include "shared/ui/advance_panel_task_x.inc"

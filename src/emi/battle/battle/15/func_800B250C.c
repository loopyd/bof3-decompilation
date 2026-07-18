#include "internal.h"

/* @source 0x800B250C
 * @behavior advances the local panel task X position by 32 pixels, clamps it
 * to 320, and clears the preceding state byte when the clamp is reached.
 */
#define BOF3_PANEL_TASK_FUNCTION func_800B250C
#define BOF3_PANEL_LIMIT 320
#include "bof3/duplicates/advance_panel_task_x.inc"

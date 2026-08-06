#include "internal.h"

/* @behavior resets scratch-state offsets 0x09 and 0x01.
 * @source 0x801F3288
 */
void world00_area008_reset_scratch_select_mode1(void)
{
  g_world00_area008_work->unk_09 = 0;
  g_world00_area008_work->mode = 1;
}

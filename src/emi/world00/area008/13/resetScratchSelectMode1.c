#include "internal.h"

/* @behavior resets scratch-state offsets 0x09 and 0x01.
 * @source 0x801F3288
 */
void resetScratchSelectMode1(void)
{
  g_areaWork->unk_09 = 0;
  g_areaWork->mode = 1;
}

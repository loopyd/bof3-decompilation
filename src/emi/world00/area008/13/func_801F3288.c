#include "internal.h"

/* @behavior resets scratch-state offsets 0x09 and 0x01.
 * @source 0x801F3288
 */
void func_801F3288(void)
{
  D_1F800044->unk_09 = 0;
  D_1F800044->mode = 1;
}

#include "internal.h"

/**
 * @source 0x801DC838
 * @behavior Appends the dim tile, then dispatches the scratch work-record mode.
 */
void func_801DC838(void)
{
  appendDimTile();
  D_801E22D0[D_1F800044[3]]();
}

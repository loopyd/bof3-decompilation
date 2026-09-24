#include "bof3/world/area03004_internal.h"

/* @source 0x801DC7EC */
/* @behavior Initializes three scratch work-record bytes when the shared mode is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The first store goes through the fixed-address scratch-cursor view, not the
 * named D_1F800044 symbol: only that form puts the stored constant in the bnez
 * delay slot (li v0,1 ahead of the folded cursor load), the lever recorded by
 * siblings func_801D11C0 and func_801DBEDC in this target. */
void func_801DC7EC(void)
{
  if (D_80143C40 == 0) {
    WORLD00_AREA030_SCRATCH_PTR[2] = 1;
    D_1F800044[3] = 0;
    D_1F800044[4] = 0;
  }
}

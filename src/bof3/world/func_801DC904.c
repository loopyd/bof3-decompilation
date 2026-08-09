#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801DC904
 * @behavior Increments scratch-record byte 3 while state byte 0x80144281 is 3.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801DC904(void)
{
  u8* record;

  if (PSX_REF(u8, 0x80144281u) == 3) {
    record = WORLD00_AREA030_SCRATCH_PTR;
    record[3]++;
  }
}

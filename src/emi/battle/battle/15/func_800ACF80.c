#include "internal.h"

/**
 * @source 0x800ACF80
 * @behavior Clear five leading state bytes in each of eight battle records.
 */
void func_800ACF80(void)
{
  u8 i;

  for (i = 0; i < 8; i++) {
    D_801EB630[i].unk_00 = 0;
    D_801EB630[i].unk_01 = 0;
    D_801EB630[i].unk_02 = 0;
    D_801EB630[i].unk_03 = 0;
    D_801EB630[i].unk_04 = 0;
  }
}

#include "internal.h"

/**
 * @source 0x801E0394
 * @behavior Advances the active scratch work record while its timer is nonzero.
 */
void func_801E0394(void)
{
  u8* first;
  u8 timer;

  first = D_1F800044;
  timer = first[9];
  if (timer != 0) {
    u8* work;

    first[9] = timer - 1;
    work = D_1F800044;
    *(s32*)(work + 0x34) += *(s32*)(work + 0x0C);
    *(s32*)(work + 0x38) += *(s32*)(work + 0x10);
    *(u16*)(work + 0x3E) += *(s32*)(work + 0x14);
  }
}

#include "internal.h"

/**
 * @source 0x801DDCF8
 * @behavior Advances the area counter and seeds two scratch-record bytes while
 * the shared state is clear.
 */
void func_801DDCF8(void)
{
  u8* counter;

  if (D_80143C40 == 0) {
    counter = &D_8014403D;
    (*counter)++;
    D_1F800044[2] = 1;
    D_1F800044[3] = 0;
  }
}

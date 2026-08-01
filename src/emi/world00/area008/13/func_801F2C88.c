#include "internal.h"

extern void func_8014D6B8(u32 flag);

/* @behavior Selects disabled mode when the shared high-bit flag is set;
 * otherwise installs and enables the area resource, then selects mode 2.
 * @source 0x801F2C88
 */
void func_801F2C88(void) {
  volatile World00Area008State* previous;

  if ((WORLD00_AREA008_D_80146867 & 0x80u) != 0u) {
    D_1F800044->mode = 9;
    return;
  }

  previous = D_1F800044;
  D_80146250 = &D_80145FD0;
  D_1F800044 = &D_80145FD0;
  D_801460E8 |= 0x40;
  func_8014D6B8(0x10);
  D_80146866 = 1;
  D_1F800044 = previous;
  previous->mode = 2;
}

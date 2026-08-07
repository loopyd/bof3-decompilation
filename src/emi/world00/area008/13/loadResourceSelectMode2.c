#include "internal.h"

extern void func_8014D6B8(u32 flag);

/* @behavior Selects disabled mode when the shared high-bit flag is set;
 * otherwise installs and enables the area resource, then selects mode 2.
 * @source 0x801F2C88
 */
void loadResourceSelectMode2(void) {
  volatile World00Area008State* previous;

  if ((D_80146867 & 0x80u) != 0u) {
    g_areaWork->mode = 9;
    return;
  }

  previous = g_areaWork;
  currentState = &areaState;
  g_areaWork = &areaState;
  D_801460E8 |= 0x40;
  func_8014D6B8(0x10);
  D_80146866 = 1;
  g_areaWork = previous;
  previous->mode = 2;
}

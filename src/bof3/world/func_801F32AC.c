#include "bof3/world/area00813_internal.h"

/**
 * @source 0x801F32AC
 * @behavior advances the area mode or clears the active entity flag.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F32AC(void) {
  World00Area008State* state;
  World00Area008Entity* entity;

  if (D_80146867 & 0x80) {
    g_areaWork->mode = 14;
    return;
  }

  if (D_80146866 == 1) {
    state = g_areaWork;
    entity = &D_80146888[state->entityIndex];
    entity->flags &= 0xbf;
    g_areaWork->mode = 2;
  }
}

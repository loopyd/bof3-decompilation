#include "bof3/world/area00813_internal.h"

/**
 * @source 0x801F32AC
 * @behavior advances the area mode or clears the active entity flag.
 */
void func_801F32AC(void) {
  World00Area008State* state;
  World00Area008Entity* entity;
  u8 entityIndex;

  if (D_80146867 & 0x80) {
    state = g_areaWork;
    state->mode = 14;
    return;
  }

  if (D_80146866 == 1) {
    state = g_areaWork;
    entityIndex = state->entityIndex;
    entity = &D_80146888[entityIndex];
    entity->flags &= 0xbf;
    state = g_areaWork;
    state->mode = 2;
  }
}

#include "bof3/world/area00813_internal.h"

/* @source 0x801F365C
 * @behavior installs the entity selected by the scratch state entity index as
 *           the scratch work pointer, selects mode 14 when the shared high-bit
 *           flag is set, or mode 8 when that entity reports halfword 0x58 equal
 *           to 8 and byte 0x4A equal to 1; finally restores the previous
 *           scratch work pointer.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 32/32 instructions, 128 bytes, live byte match.
 */
void func_801F365C(void) {
  World00Area008State* state;
  World00Area008Entity* entity;

  state = g_areaWork;
  entity = &D_80146888[state->entityIndex];
  g_areaWork = (World00Area008State*)entity;
  if (D_80146867 & 0x80) {
    state->mode = 14;
  } else if (entity->unk_58 == 8 && entity->unk_4A == 1) {
    state->mode = 8;
  }
  g_areaWork = state;
}

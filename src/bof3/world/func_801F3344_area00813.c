#include "bof3/world/area00813_internal.h"

extern void func_8014D6B8(u32 flag);

/* @source 0x801F3344
 * @behavior installs the entity selected by the scratch state entity index as
 *           the scratch work pointer, then selects mode 14 when the shared
 *           high-bit flag is set, mode 3 after requesting resource 3 when the
 *           shared secondary state byte reads 2, or mode 10 after requesting
 *           resource 3 when that byte reads 5; finally restores the previous
 *           scratch work pointer.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 43/43 instructions, 172 bytes, live byte match.
 */
void func_801F3344(void) {
  World00Area008State* state;
  World00Area008Entity* entity;

  state = g_areaWork;
  entity = &D_80146888[state->entityIndex];
  g_areaWork = (World00Area008State*)entity;
  if (D_80146867 & 0x80) {
    state->mode = 14;
  } else if (D_80146866 == 2) {
    func_8014D6B8(3);
    state->mode = 3;
  } else if (D_80146866 == 5) {
    func_8014D6B8(3);
    state->mode = 10;
  }
  g_areaWork = state;
}

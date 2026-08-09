#include "bof3/world/area03004_internal.h"

/* @behavior when the shared transition is idle, clears companion state,
 * advances the scratch work-record phase, and selects state 3.
 * @source 0x801DDC54
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801DDC54(void) {
  u8* work;

  if (D_80143C40 == 0u) {
    work = WORLD00_AREA030_SCRATCH_PTR;
    PSX_REF(u8, 0x800F724Cu) = 0u;
    PSX_REF(u8, 0x800F724Du) = 0u;
    PSX_REF(u8, 0x800F724Eu) = 0u;
    PSX_REF(u8, 0x800F724Fu) = 0u;
    D_8014832E = 0u;
    D_80143B92 = 3u;
    work[3] = (u8)(work[3] + 1u);
  }
}

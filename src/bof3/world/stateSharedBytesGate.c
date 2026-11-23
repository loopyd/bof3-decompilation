#include "bof3/world/area03004_internal.h"

/* @behavior When shared halfword 0x80143C40 is zero, clears bytes 0x800F724C
 * through 0x800F724F and 0x8014832E, writes halfword 3 at 0x80143B92
 * and advances work step byte 3.
 * @source 0x801DDC54
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateSharedBytesGate(void) {
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

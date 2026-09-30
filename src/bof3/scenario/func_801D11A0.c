#include "bof3/scenario/sce10eff_internal.h"

/* @behavior advances scratch byte 11 and counts down scratch byte 10.
 * When that countdown reaches zero it advances byte 11 of the object cached at
 * 0x801D2744 and calls func_80196070.
 * @source 0x801D11A0
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D11A0(void) {
  u8* work;
  u32 slot_offset;

  slot_offset = 0x44u;
  work = PSX_REF(u8*, SPAD_BASE + slot_offset);
  work[0x0b]++;

  work = PSX_REF(u8*, SPAD_BASE + slot_offset);
  work[0x0a]--;
  if (work[0x0a] == 0u) {
    work = D_801D2744;
    work[0x0b]++;
    func_80196070();
  }
}

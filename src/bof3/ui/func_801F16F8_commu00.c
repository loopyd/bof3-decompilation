#include "bof3/ui/commu00_internal.h"

/* @source 0x801F16F8
 * @behavior Unless the shared frontend phase byte is already 2, advances the
 * fairy progress byte when the shared flag word 0x801490A8 equals 0xF9,
 * otherwise requests action 0xF8 and latches the phase byte to 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F16F8(void) {
  u8* state = &D_80143BB0;

  if (*state == 2) {
    return;
  }
  if (D_801490A8 == 0xF9) {
    fairyProgress[0] += 1;
  } else {
    func_80150224(0xF8);
    *state = 2;
  }
}

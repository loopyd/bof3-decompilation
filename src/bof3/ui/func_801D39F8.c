#include "bof3/ui/shop00_internal.h"

/* @source 0x801D39F8
 * @behavior UI phase step: reads the phase byte D_80148651, re-arms phaseTimer,
 *           writes the incremented phase byte back, clears the overlay flag
 *           byte D_801E60F0 and the sub-step counter D_80148652, then for the
 *           selection values 0x40 (D_80143F02) and 0xBC/0x85/0xC1 (D_80143F00)
 *           archives the byte in D_8014865C into D_8014865F and sets
 *           D_8014865C to -2. When D_8014865C then holds -2 the phase byte is
 *           forced to 3 and D_80148652 is cleared again.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D39F8(void) {
  u8 phase;

  phase = D_80148651;
  phaseTimer = 4;
  D_80148651 = phase + 1;
  D_801E60F0 = 0;
  D_80148652 = 0;
  if (D_80143F02 == 0x40 || D_80143F00 == 0xBC || D_80143F00 == 0x85 ||
      D_80143F00 == 0xC1) {
    u8 saved;

    saved = D_8014865C;
    D_8014865C = -2;
    D_8014865F = saved;
  }
  if (D_8014865C == -2) {
    D_80148651 = 3;
    D_80148652 = 0;
  }
}

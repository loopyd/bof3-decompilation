#include "bof3/ui/shop00_internal.h"

/* @source 0x801D60B0
 * @behavior shop phase step: emits the shop panel strip through the panel strip
 *           emitter func_801DAB90 with the panel x constant 0x14, the
 *           phase-relative second argument (0x12 - phaseTimer * 20) masked with
 *           0xFFFE, the height constant 0x118, the constant 0x13 and the
 *           main-RAM CLUT-bank byte D_80144952 as the fifth argument. It then
 *           advances the frame timer phaseTimer by one and, when the advanced
 *           byte reaches 5, arms the write-only byte D_80148650 with 1, clears
 *           the UI phase byte D_80148651 and, for the selection values 0x40
 *           (D_80143F02) and 0xBC/0x85/0xC1 (D_80143F00), archives the overlay
 *           byte D_8014865F into the saved-state byte D_8014865C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D60B0(void) {
  volatile u8* timer;
  u8 next;

  /* The original materializes the timer address once and reuses that base for
   * the strip-coordinate load, the advance load and the advance store
   * (lui+addiu / lbu / sb through one callee-saved register). */
  timer = &phaseTimer;
  func_801DAB90(0x14, (0x12 - *timer * 20) & 0xFFFE, 0x118, 0x13, D_80144952);
  next = *timer + 1;
  *timer = next;
  if (next == 5) {
    D_80148650 = 1;
    D_80148651 = 0;
    if (D_80143F02 == 0x40 || D_80143F00 == 0xBC || D_80143F00 == 0x85 ||
        D_80143F00 == 0xC1) {
      D_8014865C = D_8014865F;
    }
  }
}

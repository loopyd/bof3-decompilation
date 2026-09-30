#include "bof3/ui/shop00_internal.h"

/* @source 0x801D7080
 * @behavior clears the 0x40-byte main-RAM text buffer D_80145AD4, then formats
 *           the shop header row through sprintf into that buffer with the
 *           EMI-local format string D_801D0C98 and the seven digit strings of
 *           the char-pointer table D_801E5434: the bracketed slot selected by
 *           the byte D_801E6108, then the tens and units halves of the
 *           front-end clock hour byte D_80144FC0, of the minute byte
 *           D_80144FC1 and of the main-RAM level byte D_8014496E.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D7080(void) {
  u8 i;

  for (i = 0; i < 0x40; i++) {
    D_80145AD4[i] = 0;
  }
  /* The original reads the hour byte once and reuses its quotient for both
   * halves, but loads the minute and level bytes twice (once per half). Naming
   * the same symbol in both halves of those two pairs lets CSE collapse the
   * second load; the units halves therefore reach the byte through an
   * address-derived byte alias, which reproduces the original's two `lbu`
   * accesses per cell at the same width and with no additional side effects. */
  sprintf((char*)D_80145AD4, D_801D0C98, D_801E5434[D_801E6108],
          D_801E5434[D_80144FC0 / 10], D_801E5434[D_80144FC0 % 10],
          D_801E5434[D_80144FC1 / 10],
          D_801E5434[PSX_REF(u8, (u32)&D_80144FC1) % 10],
          D_801E5434[D_8014496E / 10],
          D_801E5434[PSX_REF(u8, (u32)&D_8014496E) % 10]);
}

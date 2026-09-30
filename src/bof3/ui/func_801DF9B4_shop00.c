#include "bof3/ui/shop00_internal.h"

/* @source 0x801DF9B4
 * @behavior calls func_801E0F78 (shop list/state rebuild), then advances the
 *           UI phase byte D_80148651 and clears the overlay-local byte
 *           D_801E6260.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DF9B4(void) {
  func_801E0F78();
  /* The original increments the phase byte through a non-volatile, register-held
   * address: its store is scheduled past the epilogue `lw ra`, where a volatile
   * access stays pinned ahead of it (sibling shape: func_80096AB0). */
  PSX_REF(u8, (u32)&D_80148651) += 1;
  D_801E6260 = 0;
}

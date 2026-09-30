#include "bof3/world/area03005_internal.h"

/* @behavior Enters the AREA030/05 local frontend mode 1 (companion helper
 * 0x8014ECAC), clears the entry-flag byte at 0x800F723C, sets the mode byte at
 * 0x800F7244, stores the companion record index returned by func_800F66D4 in
 * the byte slot at 0x800F7240, then advances the handlerPhase byte.
 * @source 0x800F50B4
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800F50B4(void) {
  func_8014ECAC(1);
  D_800F723C = 0;
  D_800F7244 = 1;
  D_800F7240 = func_800F66D4();
  handlerPhase = handlerPhase + 1;
}

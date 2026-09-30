#include "bof3/world/area03005_internal.h"

/* @behavior Second phase entry of the AREA030/05 companion dispatch
 * (D_800F71FC[1], reached through func_800F5048's handlerTable[3 + phase]);
 * when the shared gate halfword D_80143C40 is clear it selects the primary
 * handler entry (handlerIndex = 1) and restarts the phase at handlerPhase = 0,
 * otherwise it leaves both bytes untouched and returns.
 * @source 0x800F510C
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800F510C(void) {
  if (D_80143C40 == 0) {
    handlerIndex = 1;
    handlerPhase = 0;
  }
}

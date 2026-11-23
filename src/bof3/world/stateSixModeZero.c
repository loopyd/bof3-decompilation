#include "bof3/world/area03004_internal.h"

/* @behavior While modeByte is zero, call func_8015D4F8(0, 0, 0x14, 0x10), request cue
 * 0x102, increment D_80144281, clear work byte 6 and advance work handler byte
 * 3.
 * @source 0x801DC888
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateSixModeZero(void) {
  if (modeByte == 0) {
    func_8015D4F8(0, 0, 0x14, 0x10);
    func_8015DF18(0x102);
    D_80144281++;
    D_1F800044[6] = 0;
    D_1F800044[3]++;
  }
}

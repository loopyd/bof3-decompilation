#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 mode-3 handler selected by the overlay's handler pointer
 * table: while the shared selection gate byte D_80144199_BYTE holds 3, it
 * requests state 3 from the target-local state helper func_801DDE94 and
 * advances byte 3 of the scratch work-record cursor at 0x1F800044; the shared
 * helper func_8014D978 is called unconditionally afterwards and neither result
 * is used here.
 * @source 0x801DC514
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DC514(void) {
  if (D_80144199_BYTE == 3) {
    func_801DDE94(3);
    D_1F800044[3]++;
  }
  func_8014D978();
}

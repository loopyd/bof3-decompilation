#include "bof3/world/area03004_internal.h"

/* @behavior advances one step when the shared query reports a nonzero byte:
 * starts effect 3, bumps the dispatch index, then runs the shared ready helper
 * on both paths.
 * @source 0x801D25F0
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D25F0(void) {
  if ((u8)func_8014daec() != 0u) {
    u8* cursor;

    func_8014d6b8(3);
    cursor = (u8*)D_1F800044;
    cursor[1] = (u8)(cursor[1] + 1u);
  }
  func_8014d290();
}

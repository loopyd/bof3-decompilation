#include "bof3/world/area03004_internal.h"

/* @behavior decrements the scratch countdown byte at offset 9; when the fresh
 * value reaches zero it advances the dispatch index, then runs the shared ready
 * helper on both paths.
 * @source 0x801D3510
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D3510(void) {
  u8* cursor;
  u8* next;

  cursor = (u8*)D_1F800044;
  cursor[9u] = (u8)(cursor[9u] - 1u);
  next = (u8*)D_1F800044;
  if (next[9u] == 0u) {
    next[1u] = (u8)(next[1u] + 1u);
  }
  func_8014d4e0();
}

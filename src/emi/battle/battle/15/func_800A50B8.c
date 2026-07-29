#include "internal.h"

/* @behavior Initializes battle menu state 0x15 and advances its update counter.
 * @source 0x800A50B8
 */
void func_800A50B8(void) {
  u8 *counter;
  u8 count;

  func_80158DB8(0x15, 5);
  D_80148628 = 0x142;
  D_80148626 = 0;
  D_80148627 = 0;
  D_8014862A = 0x3f;
  counter = (u8 *)&D_801462E4;
  count = *counter;
  D_801485BB = 2;
  *counter = (u8)(count + 1);
}

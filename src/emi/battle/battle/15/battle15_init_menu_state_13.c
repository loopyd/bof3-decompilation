#include "internal.h"

/* @behavior Initializes battle menu state 0x13 and advances its update counter.
 * @source 0x800A602C
 */
void battle15_init_menu_state_13(void) {
  u8 *counter;
  u8 count;

  D_801485BB = 3;
  func_80158DB8(0x13, 5);
  D_801485DE = 4;
  D_801485DF = 0;
  D_801485E0 = 0x15b;
  counter = (u8 *)&D_801462E4;
  count = *counter;
  D_801485E2 = 0x3f;
  D_801485EC = 0;
  D_801485EE = 0;
  *counter = (u8)(count + 1);
}

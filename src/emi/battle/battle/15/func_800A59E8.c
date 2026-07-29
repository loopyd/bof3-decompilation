#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @sets D_80148627 to 2, D_8014862E to 1, increments counter at D_801462E4.
 * @source 0x800A59E8
 */
void func_800A59E8(void) {
  u8 *counter = (u8 *)&D_801462E4;
  u8 value;

  D_80148627 = 2;
  value = *counter;
  D_8014862E = 1;
  *counter = value + 1;
}

#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @sets D_80148627 to 2, clears D_8014862E, increments counter at D_801462E4.
 * @source 0x800A4C44
 */
void func_800A4C44(void) {
  D_80148627 = 2;
  D_8014862E = 0;
  (*D_801462E4)++;
}

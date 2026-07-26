#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @sets D_80148627 to 2, D_8014862E to 1, increments counter at D_801462E4.
 * @source 0x800A59E8
 */
void func_800A59E8(void) {
  D_80148627 = 2;
  D_8014862E = 1;
  (*D_801462E4)++;
}

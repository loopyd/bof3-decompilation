#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @sets D_80148597 to 2, D_801483C3 to 1, increments counter at D_801462E4.
 * @source 0x800A5300
 */
void func_800A5300(void) {
  D_80148597 = 2;
  D_801483C3 = 1;
  (*D_801462E4)++;
}

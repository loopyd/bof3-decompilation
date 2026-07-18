#include "internal.h"

/* @sets D_80148627 to 2 and increments the global counter at D_801462E4.
 * @source 0x800A4B08
 */
void func_800A4B08(void) {
  D_80148627 = 2;
  (*D_801462E4)++;
}

#include "internal.h"

/* @behavior Updates two bytes through the scratchpad work-area pointer.
 * @source 0x800AE06C
 */
void func_800AE06C(void) {
  g_battle_work[9] = 0x3C;
  g_battle_work[1]++;
}

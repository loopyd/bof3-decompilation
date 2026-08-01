#include "internal.h"

/* @behavior Dispatches the handler selected by scratchpad work byte 1.
 * @source 0x800AE34C
 */
void func_800AE34C(void) {
  D_800B6BF4[((volatile u8*)g_battle_work)[1]]();
}

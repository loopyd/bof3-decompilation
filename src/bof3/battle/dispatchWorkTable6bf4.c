#include "bof3/battle/battle15_internal.h"

/* @behavior Dispatches the handler selected by scratchpad work byte 1.
 * @source 0x800AE34C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchWorkTable6bf4(void) {
  D_800B6BF4[((volatile u8*)g_battle_work)[1]]();
}

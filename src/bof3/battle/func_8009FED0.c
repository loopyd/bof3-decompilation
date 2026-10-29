#include "bof3/battle/battle15_internal.h"

/* @source 0x8009FED0
 * @behavior Stores 2 in byte +0x08 through the pointer loaded from 0x801463A0, then
 * reloads that pointer and stores -40 in the halfword at byte +0x06.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009FED0(void) {
  ((volatile u8*)D_8014639C.record)[8] = 2;
  D_8014639C.record[3] = -0x28;
}

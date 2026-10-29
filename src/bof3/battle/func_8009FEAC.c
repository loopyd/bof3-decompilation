#include "bof3/battle/battle15_internal.h"

/* @source 0x8009FEAC
 * @behavior Stores 2 in byte +0x08 through the pointer loaded from 0x801463A0, then
 * reloads that pointer and stores -5 in the halfword at byte +0x06.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009FEAC(void) {
  ((u8*)D_8014639C.record)[8] = 2;
  *(s16*)((u8*)D_8014639C.record + 6) = -5;
}

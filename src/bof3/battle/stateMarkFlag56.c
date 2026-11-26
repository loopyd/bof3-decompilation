#include "bof3/battle/battle15_internal.h"

/* @source 0x8009F6B4
 * @behavior Stores 4 into battle state byte D_80146375 and 0x56 into battle
 * state halfword D_801463C0, then calls setIndexedFlag200.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateMarkFlag56(void) {
  D_80146375 = 4;
  D_801463C0 = 0x56;
  setIndexedFlag200();
}

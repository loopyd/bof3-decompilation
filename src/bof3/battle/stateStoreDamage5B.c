#include "bof3/battle/battle15_internal.h"

/* @source 0x8009F4BC
 * @behavior Stores 4 into battle state byte D_80146375 and 0x5B into battle
 * state halfword D_801463C0, then calls storeIndexedDamageResult.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStoreDamage5B(void) {
  D_80146375 = 4;
  D_801463C0 = 0x5B;
  storeIndexedDamageResult();
}

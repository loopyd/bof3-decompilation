#include "bof3/bof3.h"
#include "bof3/battle/battle03_internal.h"

/* @source 0x801D74D4
 * @behavior clears the battle selection flag and decrements its counter when the gate is clear.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void retreatStateWhenGateClear(void) {
  volatile u8* counter;
  u8 value;

  if (D_80143C40 == 0) {
    counter = &BATTLE_GLOBAL_BYTE_62E2;
    value = *counter;
    D_8014832E = 0;
    *counter = value - 1;
  }
}

#include "bof3/bof3.h"
#include "internal.h"

/* @source 0x801D74D4
 * @behavior clears the battle selection flag and decrements its counter when the gate is clear.
 */
void func_801D74D4(void) {
  volatile u8* counter;
  u8 value;

  if (D_80143C40 == 0) {
    counter = &BATTLE_GLOBAL_BYTE_62E2;
    value = *counter;
    D_8014832E = 0;
    *counter = value - 1;
  }
}

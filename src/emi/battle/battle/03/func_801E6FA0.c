#include "bof3/bof3.h"

/* @source 0x801E6FA0
 * @behavior clears byte one in the current scratchpad work record.
 */
void func_801E6FA0(void) {
  *(volatile u8*)(*(volatile void**)SPAD_ADDRESS(0x44u) + 1) = 0;
}

#include "bof3/bof3.h"

/* @source 0x801E1C58
 * @behavior sets two bytes in the current scratchpad work record to 2 and 0.
 */
void func_801E1C58(void) {
  *(volatile u8*)(*(volatile void**)SPAD_ADDRESS(0x44u) + 1) = 2;
  *(volatile u8*)(*(volatile void**)SPAD_ADDRESS(0x44u) + 2) = 0;
}

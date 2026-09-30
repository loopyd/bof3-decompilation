#include "bof3/bof3.h"

extern u8 D_80144F28;

void func_8015B5A8(void* flag_bank, u8 bit_index);

/* @source 0x801F4324
 * @behavior Clears bit 0x4E of the shared main-RAM flag bank at 0x80144F28
 * through the shared bit-clear helper func_8015B5A8; takes no arguments and
 * returns nothing, and its 0x18-byte frame only keeps $ra across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSharedFlagBit4EWorld04Area17213_801F4324(void) {
  func_8015B5A8(&D_80144F28, 0x4E);
}

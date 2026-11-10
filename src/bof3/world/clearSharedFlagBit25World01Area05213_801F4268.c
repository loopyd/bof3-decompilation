#include "bof3/bof3.h"

extern u8 D_80144F28; /* @source 0x80144F28 @kind data */

void func_8015B5A8(void* flag_bank, u8 bit_index);

/* @source 0x801F4268
 * @behavior Clears bit 0x25 of the shared main-RAM flag bank at 0x80144F28
 *           through the shared bit-clear helper func_8015B5A8, passing the
 *           bank base by address and the flag id as the immediate 0x25.
 *           Takes no arguments and returns nothing; its 0x18-byte frame only
 *           keeps $ra across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSharedFlagBit25World01Area05213_801F4268(void) {
  func_8015B5A8(&D_80144F28, 0x25);
}

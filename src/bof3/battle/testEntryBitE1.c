#include "bof3/battle/battle15_internal.h"

/* possible name: battle_unk0e1_bit_test
 * @behavior tests one bit in the local panel entry flag byte at offset 0xe1.
 * @source 0x8009C868
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 testEntryBitE1(volatile u8* entry, s32 bit_index) {
  return (u8)((entry[0xe1] >> bit_index) & 1U);
}

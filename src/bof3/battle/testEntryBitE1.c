#include "bof3/battle/battle15_internal.h"

/* possible name: battle_unk0e1_bit_test
 * @behavior tests one bit in the local panel entry flag byte at offset 0xe1.
 * @source 0x8009C868
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u8 testEntryBitE1(volatile u8* entry, s32 bit_index) {
  return (u8)((entry[0xe1] >> bit_index) & 1U);
}

/* possible name: battle_unk0e1_bit_modify
 * @behavior sets or clears one bit in the flag byte at offset 0xe1.
 * second entry point at 0x8009c87c
 * @source 0x8009C87C
 */
void battle_unk0e1_bit_modify(volatile u8* entry, s32 bit_index, u8 set_value) {
  if (set_value)
    entry[0xe1] |= (u8)(1U << bit_index);
  else
    entry[0xe1] &= (u8) ~(1U << bit_index);
}

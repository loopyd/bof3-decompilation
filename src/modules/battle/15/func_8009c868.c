#include "internal.h"

/* possible name: battle_local_panel_entry_flag_is_set
 * does: tests one bit in the local panel entry flag byte at offset 0xe1.
 * @source: 0x8009c868 FUN_8009c868
 */
u8 func_8009c868(volatile u8* entry, s32 bit_index) {
  return (u8)((entry[0xe1] >> bit_index) & 1);
}

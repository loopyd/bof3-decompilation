#include "internal.h"

/* @behavior clears the first five bytes of the record slot at the given
 * index, zeroing per‑slot fields in the entry table at game_front_recordTable.
 * @source 0x801960C0
 */
void game_front_clear_record(u8 record_index) {
  game_front_recordTable[record_index].flags_00 = 0;
  game_front_recordTable[record_index].unk_01 = 0;
  game_front_recordTable[record_index].unk_02 = 0;
  game_front_recordTable[record_index].unk_03 = 0;
  game_front_recordTable[record_index].unk_04 = 0;
}

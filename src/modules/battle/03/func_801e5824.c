#include "internal.h"

/* does: walks the 0x30 queued slot records, makes each active entry current,
 * stores its word at `0x74`, and dispatches through the queued-slot table.
 * @source: 0x801e5824 FUN_801e5824
 */
void func_801e5824(void) {
  Battle03Handler table[4];
  u8              index;

  table[0] = BATTLE_QUEUED_SLOT_TABLE[0];
  table[1] = BATTLE_QUEUED_SLOT_TABLE[1];
  table[2] = BATTLE_QUEUED_SLOT_TABLE[2];
  table[3] = BATTLE_QUEUED_SLOT_TABLE[3];

  index = 0u;
  do {
    volatile Battle03QueuedSlot* slot;

    slot = &BATTLE_QUEUED_SLOT_ARRAY[index];
    if (slot->unk_00 != 0u) {
      BATTLE_CURRENT_QUEUED_WORD_4B20 = slot->unk_74;
      BATTLE_LOCAL_SCRATCH_PTR = (volatile Battle03LocalWork*)slot;
      BATTLE_CURRENT_QUEUED_SLOT_PTR = slot;
      table[slot->unk_06]();
    }
    index += 1u;
  } while (index < 0x30u);
}

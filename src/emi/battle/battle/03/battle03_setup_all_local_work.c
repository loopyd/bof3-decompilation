#include "internal.h"

/* @behavior iterates the three local work records, making each one current and
 * running the shared setup helper on it.
 * @source 0x801DECE0
 */
void battle03_setup_all_local_work(void) {
  Battle03LocalWork* battle_work;
  Battle03LocalWork* next_battle_work;

  battle_work = &D_80145E90;
  D_80146250 = battle_work;
  D_1F800044 = battle_work;
  battle03_dispatch_local_state_table();

  next_battle_work = battle_work + 1;
  D_80146250 = next_battle_work;
  D_1F800044 = next_battle_work;
  battle03_dispatch_local_state_table();
  battle_work = next_battle_work + 1;

  D_80146250 = battle_work;
  D_1F800044 = battle_work;
  battle03_dispatch_local_state_table();
}

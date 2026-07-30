#include "internal.h"

/* @behavior iterates the three local work records, making each one current and
 * running the shared setup helper on it.
 * @source 0x801DECE0
 */
void func_801DECE0(void) {
  Battle03LocalWork* battle_work;
  Battle03LocalWork* next_battle_work;

  battle_work = &D_80145E90;
  D_80146250 = battle_work;
  D_1F800044 = battle_work;
  func_801DEEB4();

  next_battle_work = battle_work + 1;
  D_80146250 = next_battle_work;
  D_1F800044 = next_battle_work;
  func_801DEEB4();
  battle_work = next_battle_work + 1;

  D_80146250 = battle_work;
  D_1F800044 = battle_work;
  func_801DEEB4();
}

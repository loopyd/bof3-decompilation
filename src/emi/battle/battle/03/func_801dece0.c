#include "internal.h"

extern volatile Battle03LocalWork* DAT_1f800044;
extern volatile Battle03LocalWork* DAT_80146250;
extern Battle03LocalWork           DAT_80145e90;

/* @behavior iterates the three local work records, making each one current and
 * running the shared setup helper on it.
 * @source 0x801dece0 FUN_801dece0
 */
void func_801dece0(void) {
  Battle03LocalWork* battle_work;
  Battle03LocalWork* next_battle_work;

  battle_work = &DAT_80145e90;
  DAT_80146250 = battle_work;
  DAT_1f800044 = battle_work;
  func_801deeb4();

  next_battle_work = battle_work + 1;
  battle_work = next_battle_work + 1;
  DAT_80146250 = next_battle_work;
  DAT_1f800044 = next_battle_work;
  func_801deeb4();

  DAT_80146250 = battle_work;
  DAT_1f800044 = battle_work;
  func_801deeb4();
}

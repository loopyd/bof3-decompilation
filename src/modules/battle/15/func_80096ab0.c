#include "internal.h"

/* does: stages attack-name message slot 0 into the local queue through
 * `battle_stage_attack_name_message`, then advances the top-level selection
 * state byte.
 * @source: 0x80096ab0 FUN_80096ab0
 */
void func_80096ab0(void) {
  u8* selection_root_state;

  battle_stage_attack_name_message(0, 0);
  selection_root_state = &((u8*)0x80140000)[0x62e3];
  *selection_root_state = *selection_root_state + 1;
}

#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x80097FE8
 * @behavior Initializes the active battle selection state and dimensions.
 */
void func_80097FE8(void) {
  BattleSelectionState *selection;
  u8 *state;
  u8 *active;
  u8 value;
  u8 eight;

  selection = &D_80148570;
  eight = 8;
  selection->first = 1;
  state = D_801EBF08;
  active = D_801EB4D8;
  selection->unk_01 = eight;
  selection->unk_02 = 2;
  selection->unk_03 = 2;
  selection->unk_0B = 2;
  value = state[5];
  selection->unk_0D = 0xFF;
  selection->unk_08 = 2;
  selection->unk_09 = 0;
  selection->unk_0A = value;
  selection->unk_04 = 320;
  selection->unk_06 = 63;
  selection->unk_14 = *(u32 *)(active + 0x10) & 2;
  D_80148656 = state[5];
  selection->field_6C = 0;
  D_801485DD = eight;
  D_801485DE = 0;
}

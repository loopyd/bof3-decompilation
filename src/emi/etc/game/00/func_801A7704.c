#include "internal.h"

extern u32  D_80144E88[];
extern u32* D_8014686C;

/* @behavior stores a scenario index, clears adjacent state, requests SCENA[index],
 * waits for loader readiness, then dispatches into the scenario-local jump
 * table.
 * @source 0x801A7704
 */
void func_801A7704(u8 scenario_index) {
  GameScenarioState* state;
  u8*                base;
  s32                index8;
  u16                none_sel;
  u8                 ready_phase;

  none_sel = 0xffffu;
  state = &GAME_SCENARIO_STATE;
  state->scenario_id = (s8)scenario_index;
  index8 = (s32)((s8)scenario_index) * 8;
  base = (u8*)D_80144E88;
  state->field_02 = 0u;
  state->field_03 = 0u;
  state->field_04 = 0u;
  state->field_06 = 0u;
  state->field_05 = 0u;
  state->field_01 = 0u;
  state->field_08 = 0u;

  *(u32*)(base + index8) = 0u;
  D_8014686C = (u32*)(base + ((s32)state->scenario_id * 8));
  ready_phase = 5u;
  func_801A7804();

  while (!emi_loader_is_ready()) {
    if ((D_80143F00 != none_sel) && (D_80143BB0 != ready_phase)) {
      func_801992B8();
    }
    func_8014B87C(1u);
  }

  func_801A782C();
}

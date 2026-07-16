#include "internal.h"

struct ScenarioState {
  u8  pad[0x6870];
  s8  scenario_id;
  u8  field_6871;
  u8  field_6872;
  u8  field_6873;
  u8  field_6874;
  u8  field_6875;
  u16 field_6876;
  u16 field_6878;
};

#define SCENARIO_STATE ((struct ScenarioState*)0x80140000u)

/* @behavior stores a scenario index, clears adjacent state, requests SCENA[index],
 * waits for loader readiness, then dispatches into the scenario-local jump
 * table.
 * @source 0x801A7704
 */
void func_801A7704(u8 scenario_index) {
  s8*  scenario_state;
  s32  stored_index;
  u32* scenario_record;

  scenario_state = &SCENARIO_STATE->scenario_id;
  scenario_state[0] = scenario_index;
  scenario_state[2] = 0u;
  scenario_state[3] = 0u;
  scenario_state[4] = 0u;
  *(u16*)(scenario_state + 6) = 0u;
  scenario_state[5] = 0u;
  scenario_state[1] = 0u;
  *(u16*)(scenario_state + 8) = 0u;

  *(u32*)((u8*)0x80144e88u + ((s8)scenario_index * 8)) = 0u;
  stored_index = scenario_state[0];
  scenario_record = (u32*)((u8*)0x80144e88u + (stored_index * 8));
  *(u32*)0x8014686cu = (u32)scenario_record;
  func_801A7804();

  while (!emi_loader_is_ready()) {
    if ((D_80143F00 != 0xffffu) && (D_80143BB0 != 5u)) {
      func_801992B8();
    }
    func_8014B87C(1u);
  }

  func_801A782C();
}

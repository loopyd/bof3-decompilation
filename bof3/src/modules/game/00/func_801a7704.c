#include "internal.h"

/* does: stores a scenario index, clears adjacent state, requests SCENA[index],
 * waits for loader readiness, then dispatches into the scenario-local jump
 * table.
 * @source: 0x801a7704 FUN_801a7704
 */
void func_801a7704(u8 scenario_index) {
  volatile u8*  scenario_state;
  volatile u32* scenario_record;

  scenario_state = (volatile u8*)0x80146870u;
  scenario_state[2] = 0u;
  scenario_state[3] = 0u;
  scenario_state[4] = 0u;
  *(volatile u16*)(scenario_state + 6) = 0u;
  scenario_state[5] = 0u;
  scenario_state[1] = 0u;
  *(volatile u16*)(scenario_state + 8) = 0u;
  scenario_state[0] = scenario_index;

  scenario_record = (volatile u32*)(0x80144e88u + ((u32)scenario_index * 8u));
  scenario_record[0] = 0u;
  *(volatile u32*)0x8014686cu = (u32)scenario_record;
  func_801a7804();

  while (!func_80162d00()) {
    if ((BOF3_GAME_WORLD_STATE != 0xffffu) &&
        (BOF3_GAME_ENTRY0_WORLD_PHASE != 5u)) {
      func_801992b8();
    }
    func_8014b87c(1u);
  }

  func_801a782c();
}

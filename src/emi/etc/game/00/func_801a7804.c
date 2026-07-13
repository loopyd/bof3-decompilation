#include "internal.h"

struct ScenarioState {
  u8 pad[0x6870];
  s8 scenario_id;
};

#define SCENARIO_STATE ((struct ScenarioState*)0x80140000u)

void func_801a7804(void) {
  func_80161fdc(SCENARIO_STATE->scenario_id + 661);
}

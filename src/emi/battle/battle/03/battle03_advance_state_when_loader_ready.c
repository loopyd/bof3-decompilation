#include "internal.h"

/* @behavior advances the battle state dispatcher once the EXE-side EMI loader
 * is ready.
 * @source 0x801D6EAC
 */
void battle03_advance_state_when_loader_ready(void) {
  if (func_80162D00() != 0) {
    volatile u8* state = &BATTLE_GLOBAL_BYTE_62E2;

    *state += 1;
  }
}

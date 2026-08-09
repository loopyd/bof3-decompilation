#include "bof3/battle/battle03_internal.h"

/* @behavior reads one or two bytes from the current enemy script pointer at `0xec`,
 * forwards the low 7 bits through the shared effect helper, and stores the high
 * bit result into scratch byte `0x2a`.
 * @source 0x801E2314
 * @status partial
 * @match 14.88
 * @residual non-exact live audit: 18/121 instructions; 484 original bytes versus 220 current.
 */
void func_801E2314(u32 arg0) {
  volatile u8* script_ptr;
  u8           mode;
  u8           value;

  script_ptr = BATTLE_ENEMY_PTR_EC(BATTLE_CURRENT_ENEMY_PTR);
  mode = BATTLE_LOCAL_SCRATCH_PTR->unk_08;

  if (mode == 1u) {
    func_8014D8D4(script_ptr[arg0 & 0xffu] & 0x7fu);
    value = script_ptr[arg0 & 0xffu];
    BATTLE_LOCAL_BYTE_2A(BATTLE_LOCAL_SCRATCH_PTR) = (value & 0x80u) == 0u;
    return;
  }

  if (mode == 0u) {
    func_8014D8D4(script_ptr[arg0 & 0xffu] & 0x7fu);
    value = script_ptr[arg0 & 0xffu];
  } else if (mode == 2u || mode == 3u) {
    func_8014D8D4(script_ptr[(arg0 & 0xffu) + 1u] & 0x7fu);
    value = script_ptr[(arg0 & 0xffu) + 1u];
  } else {
    return;
  }

  BATTLE_LOCAL_BYTE_2A(BATTLE_LOCAL_SCRATCH_PTR) = (value & 0x80u) == 0u;
}

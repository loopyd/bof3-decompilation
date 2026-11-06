#include "bof3/battle/battle03_internal.h"

/* @behavior resets the front-end selection effect while the 0x80146325 bit-0x40
 * gate is clear, initializes the battle UI bundle slot 0, queues the fixed event
 * cue 0x11 into dispatch slot 0 with mode 2, and then advances the battle state
 * byte 0x801462E2. Original bytes place it at handler 0 of the D_801EAD00
 * battle-state table, handler 3 of D_801EACF4 and handler 6 of D_801EACE8.
 * @source 0x801D6E20
 * @status exact
 * @match 100.00
 * @residual none
 */
void initBattleUiAndQueueFixedCue(void) {
  u32 cue;

  if ((D_80146325 & 0x40u) == 0u) {
    func_8015D5D8();
    func_8015D300(0u, 2u, 0x64u, 0x64u);
  }
  initUiBundleSlot0();
  cue = func_801502D0(0x11u);
  func_801DE60C(0u, 2u, 0u, 0u, 0xffu, cue);
  BATTLE_GLOBAL_BYTE_62E2 += 1;
}

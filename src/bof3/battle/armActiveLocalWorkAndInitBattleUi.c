#include "bof3/battle/battle03_internal.h"

/* @source 0x801D6CC0
 * @behavior Restarts the local state chain of each of the three active party
 * work records at state 2, resets the front-end selection effect while the
 * 0x80146325 bit-0x40 gate is clear, initializes the battle UI bundle slot 0,
 * queues the opening event cue and then advances the battle state byte
 * 0x801462E2. Original bytes place it at handler 0 of the D_801EACF4
 * battle-state table and handler 3 of D_801EACE8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armActiveLocalWorkAndInitBattleUi(void) {
  if ((D_80145E90[0].flags_00 & 1u) != 0u) {
    D_80145E90[0].unk_01 = 2u;
  }
  if ((D_80145E90[1].flags_00 & 1u) != 0u) {
    D_80145E90[1].unk_01 = 2u;
  }
  if ((D_80145E90[2].flags_00 & 1u) != 0u) {
    D_80145E90[2].unk_01 = 2u;
  }
  if ((D_80146325 & 0x40u) == 0u) {
    func_8015D5D8();
    func_8015D300(0u, 1u, 0x64u, 0x64u);
  }
  initUiBundleSlot0();
  func_801DEAE0();
  BATTLE_GLOBAL_BYTE_62E2 += 1;
}

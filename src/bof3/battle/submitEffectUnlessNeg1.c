#include "bof3/battle/battle03_internal.h"

/* @behavior submits one effect id unless the caller passes `-1`.
 * @source 0x801E52F0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void submitEffectUnlessNeg1(s16 arg0) {
  u16 effect_id;

  effect_id = (u16)arg0;
  if (effect_id != 0xffffu) {
    func_8015DF18(effect_id);
  }
}

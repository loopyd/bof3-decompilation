#include "bof3/battle/battle03_internal.h"

/* @behavior submits one small positional effect around the current scratch object
 * when bit `0x80` is set in the input mask.
 * @source 0x801DDAB4
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void submitPositionalEffectBit80(u32 arg0) {
  if ((arg0 & 0x80u) != 0u) {
    func_8019651C(SPAD_PTR_SLOT(void, 0x44u), -6, -10, 0, 0);
  }
}

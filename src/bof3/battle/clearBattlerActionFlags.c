#include "bof3/battle/battle03_internal.h"

/* @behavior clears the small local/enemy action scratch flags for one battler and
 * removes it from the ranked owner list if present.
 * @source 0x801DD14C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void clearBattlerActionFlags(u8 arg0) {
  u8 arg_copy;
  u8 index;

  arg_copy = arg0;
  if (arg_copy < 3u) {
    D_80145E90[arg_copy].unk_119 = 0u;
    D_80145E90[arg_copy].unk_124 &= 0xfffffffdu;
    D_80145E90[arg_copy].unk_128 &= 0xfffffffbu;
  } else {
    D_801EB630[arg_copy - 3u].unk_f5 = 0u;
    D_801EB630[arg_copy - 3u].unk_100 &= 0xfffffffdu;
  }

  index = 0u;
  while (index < *(&BATTLE_GLOBAL_BYTE_6323)) {
    if (arg0 == (&D_8014630C)[index]) {
      (&D_8014630C)[index] = 0xffu;
    }
    index += 1u;
  }
}

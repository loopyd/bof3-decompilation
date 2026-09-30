#include "bof3/battle/battle03_internal.h"

/* @source 0x801E71EC
 * @behavior When battle global flags 0x801462E8 have bit 4 set, looks up the
 * group-flag halfword of the record selected by the current work byte +5 --
 * local work record +0x80 below three, enemy work record +0x82 (index - 3)
 * otherwise -- and then either resets scratchpad byte +1 to zero when the
 * 0x58 bits are clear or decrements it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countdownScratchByte1ByGroupFlags(void) {
  Battle03LocalWork* work;
  u8 selector;
  u16 group_flags;

  if ((BATTLE_GLOBAL_HALF_62E8 & 4u) == 0u) {
    return;
  }

  work = D_801EB4E0;
  selector = work->unk_05;

  if (selector < 3u) {
    group_flags = D_80145E90[selector].unk_80 & 0x58u;
  } else {
    group_flags = D_801EB630[selector - 3u].unk_82 & 0x58u;
  }

  if (group_flags == 0u) {
    SPAD_PTR_SLOT(u8, 0x44u)[1] = 0;
  } else {
    SPAD_PTR_SLOT(u8, 0x44u)[1]--;
  }
}

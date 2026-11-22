#include "bof3/bof3.h"

/* Shared battle flag byte four above the battle flag halfword 0x801462E8; the
 * sibling lifts of this overlay test its bits 0x2 and 0x40 and set bit 0x2, and
 * this handler tests and sets bit 0x1. */
extern u8 D_801462ED; /* @source 0x801462ED @kind unknown */

/* Global byte this handler writes with 0, 1 or 3 depending on the shared flag
 * bit and the step countdown; the sibling lift
 * storeThreeOrZeroByFlagBitThreeBossBoss02513_800C2128 clears it to 0 by another
 * flag bit. Its role is not established here. */
extern u8 D_80146375; /* @source 0x80146375 @kind unknown */

/* Global byte this handler stores 4 into when the step countdown expires; the
 * same sibling lift stores 3 into it. Its role is not established here. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Global byte this handler decrements on its fire path. Its role is not
 * established here. */
extern u8 D_80146322; /* @source 0x80146322 @kind unknown */

/* Overlay step countdown byte this payload's own data run holds: the installer
 * publishBattleHooksAndSeedStepCountdownBossBoss01316_800C20D8 seeds it from
 * rand() and this handler decrements it while it is nonzero and fires when it is
 * already zero. */
extern u8 D_800C26E0; /* @source 0x800C26E0 @kind data */

/* @source 0x800C1E90
 * @behavior Overlay step handler and entry 0 of the payload handler run at
 * 0x800C26D4 that dispatchIndexedHandlerBossBoss01316_800C1E58 scales a byte
 * index into. It reads the shared flag byte 0x801462ED once: while bit 0 is set
 * it stores 3 into the global byte 0x80146375 and returns. Otherwise, while the
 * overlay step countdown byte 0x800C26E0 is nonzero it stores one less there and
 * clears 0x80146375 to 0. When that countdown is already zero it stores 1 into
 * 0x80146375, stores 4 into the global byte 0x80146384, writes back the flag
 * byte it read at entry with bit 0 set, and stores the global byte 0x80146322
 * less one. It calls nothing, takes no arguments, has no frame and returns no
 * value. Only the run word at 0x800C26D4 holds its address, so no jal in the
 * payload names it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void storeThreeZeroOrOneByFlagBitAndCountdownThenSetFlagBitAndDecrementByte6322BossBoss01316_800C1E90(void) {
  u8* flags;
  u8 value;
  u8 countdown;
  u8 other;

  flags = &D_801462ED;
  value = *flags;
  if (value & 0x1) {
    D_80146375 = 3;
  } else {
    countdown = D_800C26E0;
    if (countdown != 0) {
      D_800C26E0 = countdown - 1;
      D_80146375 = 0;
    } else {
      D_80146375 = 1;
      D_80146384 = 4;
      other = D_80146322;
      *flags = value | 0x1;
      D_80146322 = other - 1;
    }
  }
}

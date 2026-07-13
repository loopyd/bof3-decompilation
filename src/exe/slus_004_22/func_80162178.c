#include "internal.h"

extern u8     DAT_80146480;
extern u8     DAT_8014648a;
extern s8     DAT_8014648b;
extern u16    DAT_80146490;
extern u16    DAT_80146492;
extern u8     DAT_80146494;
extern CdlLOC DAT_80146778;
extern vu32   DAT_80146808;

/* @behavior resets EMI transfer counters, converts the current LBA to CdlLOC, and
 * arms the next loader phase.
 * @source 0x80162178 FUN_80162178
 */
void func_80162178(void) {
  volatile u8* read_progress;
  s8           state;
  u32          lba;

  read_progress = &DAT_80146494;
  *read_progress = 0;
  lba = DAT_80146808;
  DAT_80146490 = 0;
  CdIntToPos(lba, (CdlLOC*)(&DAT_80146494 + 0x2e4));
  DAT_80146492 = 3;
  DAT_80146480 = 0;
  state = DAT_8014648b;
  DAT_8014648a = state == 1 ? 6 : 1;
}

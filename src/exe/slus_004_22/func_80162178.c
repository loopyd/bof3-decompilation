#include "internal.h"

extern u8     D_80146480;
extern u8     D_8014648A;
extern s8     D_8014648B;
extern u16    D_80146490;
extern u16    D_80146492;
extern u8     D_80146494;
extern CdlLOC D_80146778;
extern vu32   D_80146808;

/* @behavior resets EMI transfer counters, converts the current LBA to CdlLOC, and
 * arms the next loader phase.
 * @source 0x80162178 FUN_80162178
 */
void func_80162178(void) {
  volatile u8* read_progress;
  s8           state;
  u32          lba;

  read_progress = &D_80146494;
  *read_progress = 0;
  lba = D_80146808;
  D_80146490 = 0;
  CdIntToPos(lba, (CdlLOC*)(&D_80146494 + 0x2e4));
  D_80146492 = 3;
  D_80146480 = 0;
  state = D_8014648B;
  D_8014648A = state == 1 ? 6 : 1;
}

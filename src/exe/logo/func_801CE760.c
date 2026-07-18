#include "internal.h"

/*
 * @source 0x801CE760
 * @behavior initializes LOGO.EXE work-area globals, then starts dependent
 * subsystems and the supplied disc-LBA setup.
 */
void func_801CE760(s32 work_base, u_long disc_lba) {
  volatile u8 scratch[0x30];

  (void)scratch;
  D_801EB448 = work_base + 0x2D00;
  D_801EB444 = work_base;
  D_801EB454 = 0;
  D_801EB44C = work_base + 0xA500;
  D_801EB450 = work_base + 0x10480;
  CdReadSync(0, 0);
  PadInit(0);
  func_801CE7F4();
  func_801CE930(disc_lba);
  func_801CED48();
}

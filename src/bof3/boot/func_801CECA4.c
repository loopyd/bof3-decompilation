#include "bof3/boot/logo_internal.h"

/*
 * @source 0x801CECA4
 * @behavior MDEC output callback of the CAPCOM30.STR stream: clears the pending
 * D_801EB678 word through func_801D263C when it is set, hands the LOGO frame
 * column RECT D_801EB45C and the D_801EB444 work buffer to func_801D44D4,
 * advances the column by 0x18 and, while the column stays inside the 0x1E0
 * line, resubmits the 0xB40-byte buffer through func_801D6FCC and publishes the
 * D_801EB46C transfer flag that func_801CEC88 waits on; the flag is cleared
 * instead once the column leaves the line.
 * @status exact
 * @match 100.00
 * @residual none
 * live comparison is instruction- and byte-exact.
 */
void func_801CECA4(void) {
  u16 column;

  if (D_801EB678 != 0) {
    func_801D263C();
    D_801EB678 = 0;
  }
  func_801D44D4(&D_801EB45C, D_801EB444);
  /* The column counter is read as an unsigned halfword and re-signed for the
   * line bound, as the original does. */
  column = *(u16*)&D_801EB45C.x + 0x18;
  D_801EB45C.x = column;
  if ((s16)column < 0x1E0) {
    func_801D6FCC(D_801EB444, 0xB40);
    D_801EB46C = 1;
  } else {
    D_801EB46C = 0;
  }
}

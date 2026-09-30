#include "bof3/boot/logo_internal.h"

#include <libpress.h>

/*
 * @source 0x801CEBFC
 * @behavior tears down the CAPCOM30.STR stream: ramps the LOGO CD mixing
 * record from 0x80 down to zero, resets the CD hook state inside the critical
 * section, issues the CD helper with mode 0, submits the 0x200-byte buffer at
 * D_801EB44C (work area + 0xA500) to the CD transfer helper, sets the
 * D_801EB440 byte to 0x80 and waits until the CD dispatcher func_801D0158
 * reports completion for mode 0xE and then mode 9, and finally clears the MDEC
 * output callback.
 * @status exact
 * @match 100.00
 * @residual none
 * live comparison is instruction- and byte-exact.
 */
void func_801CEBFC(void) {
  func_801CEDA4();
  func_801D2294();
  func_801CFEA4(0, 0);
  func_801D02BC(D_801EB44C, 0x200);
  D_801EB440 = 0x80;
  do {
  } while (func_801D0158(0xE, &D_801EB440, 0) == 0);
  do {
  } while (func_801D0158(9, 0, 0) == 0);
  DecDCToutCallback(0);
}

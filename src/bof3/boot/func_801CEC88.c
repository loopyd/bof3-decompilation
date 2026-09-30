#include "bof3/boot/logo_internal.h"

/*
 * @source 0x801CEC88
 * @behavior waits for the LOGO.EXE stream transfer flag D_801EB46C to clear,
 * spinning on the flag that the func_801CE930 stream setup arms and the
 * func_801CECA4 MDEC output callback completes.
 * @status exact
 * @match 100.00
 * @residual none
 * live comparison is instruction- and byte-exact.
 */
void func_801CEC88(void) {
  do {
  } while (D_801EB46C != 0);
}

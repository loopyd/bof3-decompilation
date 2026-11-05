#include "bof3/bof3.h"

void func_801E5988(void);

extern u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EFCFC
 * @behavior Raises bit 0x4 of the shared battle global halfword at 0x801462E8
 * and then invokes the helper at 0x801E5988 unconditionally. This entry is
 * selected by work byte 1 through the code pointer at 0x801EEC48. Its bytes
 * are identical to the entries at 0x801EEFDC, 0x801EF728 and 0x801EFA40.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4AndInvokeHelperBmagicMagic00800_801EFCFC(void) {
  u16 *flags = &D_801462E8;

  *flags |= 4;
  func_801E5988();
}

#include "bof3/context.h"
#include "internal.h"

/* PsyQ LIBAPI pad shutdown; absent from the vendored libapi.h. */
extern void StopPAD2(void);
/* PsyQ jump-vector stub, resolved through the shared SDK map. */
extern void bios_JumpVector_0x8017F3D0(void);

/* @behavior shuts down pad interrupt handling during teardown.
 * @source 0x8017F1CC
 */
void func_8017F1CC(void) {
  bios_JumpVector_0x8017F3D0();
  StopPAD2();
  dequeueIntRpIrq();
}

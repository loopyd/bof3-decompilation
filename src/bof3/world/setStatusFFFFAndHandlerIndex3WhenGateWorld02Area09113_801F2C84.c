#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4286;

/* @source 0x801F2C84
 * @behavior Loads the shared gate byte at 0x801490C7 with a signed byte read
 *           (`lb`, so the cell is s8); when that byte is nonzero it stores the
 *           constant 3 into the handler index byte at 0x801D4286 (`addiu
 *           v0,0,3` in the branch delay slot, `sb`), and it then stores 0xFFFF
 *           into the shared status halfword at 0x801490A8 (`ori v0,0,0xFFFF`,
 *           `sh`) on every path. No arguments, no return value; the only
 *           writes are the conditional 0x801D4286 store and the unconditional
 *           0x801490A8 store.
 *           Constant/role evidence: the 48 bytes 0x801F2C84..0x801F2CB4 are
 *           byte-identical to the already-exact closure
 *           setStatusFFFFAndHandlerIndex3WhenGateWorld02Area09813_801F5A68,
 *           setStatusFFFFAndHandlerIndex3WhenGateWorld03Area14313_801F39E4 and
 *           setStatusFFFFAndHandlerIndex3WhenGateWorld01Area06813_801F3930,
 *           which are entries of their payload's word handler tables; the index
 *           constant 3 is corroborated by the 0x801D4286 handler-index consumer
 *           in emi/etc/sisyou/00 (exact lift src/bof3/ui/func_801D2688.c
 *           dispatches through D_801D41FC[D_801D4286]). This payload holds no
 *           in-image pointer to 0x801F2C84, so the boundary rests on the body's
 *           own `jr $ra` return and on the neighbouring Splat rows.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndHandlerIndex3WhenGateWorld02Area09113_801F2C84(void) {
  if (D_801490C7 != 0) {
    D_801D4286 = 3;
  }
  D_801490A8 = 0xFFFF;
}

#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4286;

/* @source 0x801F5A98
 * @behavior Loads the shared gate byte at 0x801490C7 with a signed byte read
 *           (`lb`, so the cell is s8); when that byte is nonzero it stores the
 *           constant 5 into the handler index byte at 0x801D4286 (`addiu
 *           v0,0,5` in the branch delay slot, `sb`), and it then stores 0xFFFF
 *           into the shared status halfword at 0x801490A8 (`ori v0,0,0xFFFF`,
 *           `sh`) on every path. No arguments, no return value.
 *           Constant/variance evidence: the 48 bytes 0x801F5A98..0x801F5AC8 are
 *           byte-identical to the already-exact closure
 *           setStatusFFFFAndHandlerIndex5WhenGateWorld03Area14313_801F3A14, and
 *           differ from the sibling selector at 0x801F5A68 only in that stored
 *           index (3 vs 5); the 0x801D4286 handler-index role is corroborated by
 *           the consumer in emi/etc/sisyou/00 (exact lift
 *           src/bof3/ui/func_801D2688.c dispatches D_801D41FC[D_801D4286]).
 *           Inside this payload both are entries of the word handler table at
 *           0x801F64B8 (func_801F5F58).
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndHandlerIndex5WhenGateWorld02Area09813_801F5A98(void) {
  if (D_801490C7 != 0) {
    D_801D4286 = 5;
  }
  D_801490A8 = 0xFFFF;
}

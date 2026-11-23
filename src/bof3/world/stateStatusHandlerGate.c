#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4286;

/* @source 0x801F2C6C
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read
 *           (`lb`, so the cell is s8) and, while that byte is nonzero, stores
 *           the constant 3 into the handler index byte D_801D4286 (`addiu
 *           $v0,$zero,3` fills the branch delay slot, then `sb`); it then
 *           stores 0xFFFF into the shared status halfword D_801490A8 (`ori
 *           $v0,$zero,0xFFFF`, `sh`) on every path, so that store runs once,
 *           after the branch joins. Takes no arguments and returns nothing; the
 *           only writes are the conditional index byte and the unconditional
 *           status halfword.
 *           Shape: the 48 bytes at 0x801F2C6C are byte-identical (payload cmp,
 *           equal from the first word through the closing `jr $ra` nop) to the
 *           recorded-exact family members
 *           setStatusFFFFAndHandlerIndex3WhenGateWorld01Area05013_801F2C60,
 *           world01/area059/13@0x801F2C48, world00/area003/13@0x801F2C48,
 *           world01/area041/13@0x801F3CD4 and world00/area037/13@0x801F34A8,
 *           which supply this body's statement order, operand forms and
 *           declaration style. The index-5 neighbour
 *           setStatusFFFFAndHandlerIndex5WhenGateWorld03Area11613_801F2C9C
 *           directly after this span is the same body differing in that one
 *           stored immediate word and nothing else.
 *           Role evidence: D_801D4286 is the shared handler index byte that the
 *           menu handler tables are dispatched through (`D_801D4240[D_801D4286]`
 *           and neighbours), and 0x801490A8/0x801490C7 are the shared status
 *           halfword and gate byte the rest of the setStatus* world family
 *           reads and writes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStatusHandlerGate(void) {
  if (D_801490C7 != 0) {
    D_801D4286 = 3;
  }
  D_801490A8 = 0xFFFF;
}

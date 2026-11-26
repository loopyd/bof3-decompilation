#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4286;

/* @source 0x801F2C9C
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read
 *           (`lb`, so the cell is s8) and, while that byte is nonzero, stores
 *           the constant 5 into the handler index byte D_801D4286 (`addiu
 *           $v0,$zero,5` fills the branch delay slot, then `sb`); it then
 *           stores 0xFFFF into the shared status halfword D_801490A8 (`ori
 *           $v0,$zero,0xFFFF`, `sh`) on every path, so that store runs once,
 *           after the branch joins. Takes no arguments and returns nothing; the
 *           only writes are the conditional index byte and the unconditional
 *           status halfword.
 *           Shape: the 48 bytes at 0x801F2C9C were byte-identical in the payload
 *           comparison (equal from the first word through the closing `jr $ra`
 *           nop) to the family members recorded exact in the tree when this seed
 *           was written -
 *           setStatusFFFFAndHandlerIndex5WhenGateWorld03Area14313_801F3A14,
 *           world01/area050/13@0x801F2C90 and world00/area003/13@0x801F2C78 -
 *           which supply this body's statement order, operand forms and
 *           declaration style. The index-3 neighbour
 *           stateStatusHandlerGate
 *           directly before this span is the same body differing in that one
 *           stored immediate word and nothing else (payload comparison: byte 17
 *           of the span only).
 *           Role evidence: D_801D4286 is the shared handler index byte the
 *           `D_801F34E8` dispatch table is indexed through (this span's address
 *           is one word of that run, at 0x801F3498), and 0x801490A8/0x801490C7
 *           are the shared status halfword and gate byte the rest of the
 *           setStatus* world family reads and writes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setHandlerIndex5Gate(void) {
  if (D_801490C7 != 0) {
    D_801D4286 = 5;
  }
  D_801490A8 = 0xFFFF;
}

#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4286;

/* @source 0x801F2CB4
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read
 *           (`lb`, so the cell is s8); while that byte is nonzero it stores the
 *           constant 5 into the handler index byte D_801D4286 (`addiu v0,0,5`
 *           in the branch delay slot, `sb`), and it then stores 0xFFFF into the
 *           shared status halfword D_801490A8 (`ori v0,0,0xFFFF`, `sh`) on
 *           every path. Takes no arguments and returns nothing; the only
 *           writes are the conditional 0x801D4286 store and the unconditional
 *           0x801490A8 store.
 *           Constant/role evidence: the 48 bytes 0x801F2CB4..0x801F2CE4 are
 *           byte-identical to the already-exact closure
 *           setStatusFFFFAndHandlerIndex5WhenGateWorld01Area05513_801F2CB8
 *           (payload offsets 0xB4 and 0xB8 of two payloads that both load at
 *           0x801F2C00, compared word for word); that level-5 sibling and this
 *           target's own exact setStatusFFFFAndHandlerIndex3WhenGate-
 *           World02Area09113_801F2C84 differ from this body only in the index
 *           constant they materialise, so the statement order and the operand
 *           forms are the sibling's and the index 5 is this span's own bytes.
 *           The 0x801D4286 handler-index byte is consumed by a dispatcher in
 *           emi/etc/sisyou/00 (exact lift src/bof3/ui/func_801D2688.c
 *           dispatches through D_801D41FC[D_801D4286]).
 *           Boundary evidence: the word at 0x801F3B38 inside this payload's
 *           pointer run 0x801F3B30.. is 0x801F2CB4 itself (address-taken entry
 *           of the ordered handler run 0x801F2C04, 0x801F2C84, 0x801F2CB4,
 *           0x801F2CE4, 0x801F2D28), and no `jal` immediate in the payload names
 *           it, so its role stays address-taken and its parameter list empty.
 *           The body ends with its own `jr $ra` and the next run entry
 *           0x801F2CE4 fixes the 0x30-byte boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateSelectHandler5Gate(void) {
  if (D_801490C7 != 0) {
    D_801D4286 = 5;
  }
  D_801490A8 = 0xFFFF;
}

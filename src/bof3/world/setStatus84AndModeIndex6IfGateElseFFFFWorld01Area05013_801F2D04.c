#include "bof3/bof3.h"

extern s8 D_801490C7;  /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
/* @source 0x801D4285 @kind bss — shared mode index: selects the handler from the
 * D_801D41E0 table and is set to 6 when an entry action starts
 * (include/bof3/ui/sisyou00_internal.h). */
extern u8 D_801D4285;

/* @source 0x801F2D04
 * @behavior Loads the shared gate byte D_801490C7 and, while that byte is
 *           nonzero, stores 0x84 into the shared status halfword D_801490A8 and
 *           6 into the shared mode index byte D_801D4285; when the gate byte is
 *           zero it stores 0xFFFF into D_801490A8 instead and touches nothing
 *           else. Takes no arguments and returns nothing; leaf with no frame,
 *           and the status constant is materialised in $v0 and fills the branch
 *           delay slot.
 *           Evidence: the 68 bytes 0x801F2D04..0x801F2D48 differ from the exact
 *           sibling setStatus4AndModeIndex6IfGateElseFFFFWorld03Area11613_801F2CCC
 *           (emi/world03/area116/13@0x801F2CCC) only in its own status literal
 *           (`addiu v0,0,0x84` vs `0x4`) and the address in its own `j`
 *           immediate; this function is entry 4 of the five-word handler table
 *           at 0x801F3CD4 inside this payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus84AndModeIndex6IfGateElseFFFFWorld01Area05013_801F2D04(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 0x84;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}

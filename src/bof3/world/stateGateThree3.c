#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4286;

/* @source 0x801F2C88
 * @behavior Reads the shared gate byte D_801490C7 and, while that byte is
 *           nonzero, stores 3 into the handler index byte D_801D4286. Every
 *           path then stores 0xFFFF into the shared status halfword D_801490A8,
 *           so that store runs once, after the branch joins. Takes no arguments
 *           and returns nothing; the 3 constant is materialised in $v0 and
 *           fills the branch delay slot.
 *           Shape: the 48 bytes at 0x801F2C88 are byte-identical, equal from the
 *           first word to the `jr $ra` nop, to the recorded-exact handler index
 *           3 members of this family at world01/area050/13@0x801F2C60,
 *           world01/area068/13@0x801F3930, world00/area003/13@0x801F2C48,
 *           world01/area041/13@0x801F3CD4 and world01/area061/13@0x801F2C50,
 *           whose body supplies this statement order, operand forms and
 *           declaration style. The handler index 5 twin at
 *           world01/area055/13@0x801F2CB8 differs in that one stored immediate
 *           alone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateGateThree3(void) {
  if (D_801490C7 != 0) {
    D_801D4286 = 3;
  }
  D_801490A8 = 0xFFFF;
}

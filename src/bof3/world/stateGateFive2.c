#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4286;

/* @source 0x801F2C90
 * @behavior Loads the shared gate byte D_801490C7; while that byte is nonzero
 *           the handler index byte at 0x801D4286 is set to 5. The shared status
 *           halfword D_801490A8 is then stored with 0xFFFF on every path, so
 *           that store runs once, after the branch joins. Takes no arguments
 *           and returns nothing; the 0x5 constant is materialised in $v0 and
 *           fills the branch delay slot, and the body is leaf with no frame.
 *           Shape: the 48 bytes at 0x801F2C90 are byte-identical (whole-span
 *           payload cmp, equal from the first word to the `jr $ra` nop) to the
 *           recorded-exact sibling
 *           setStatusFFFFAndHandlerIndex5WhenGateWorld00Area00313_801F2C78,
 *           which supplies this body's statement order, operand forms and
 *           declaration style. This function's address is word index 2 of the
 *           five-word handler table at 0x801F3CD4 inside this payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateGateFive2(void) {
  if (D_801490C7 != 0) {
    D_801D4286 = 5;
  }
  D_801490A8 = 0xFFFF;
}

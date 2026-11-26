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
 *           Shape: a payload cmp of the 48 bytes at 0x801F2C90 against
 *           out/binaries/emi/world01/area050/13.bin offset 0x90 came back equal
 *           from the first word to the `jr $ra` nop, so the recorded-exact
 *           sibling setStatusFFFFAndHandlerIndex5WhenGateWorld01Area05013_801F2C90
 *           supplied this body's statement order, operand forms and declaration
 *           style; its own comment measures the same bytes to
 *           world00/area003/13@0x801F2C78. In this payload 0x801F2C90 is word
 *           index 2 of the five-word handler pointer run at 0x801F2C04
 *           (payload bytes 0x1B8 through 0x1CB).
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

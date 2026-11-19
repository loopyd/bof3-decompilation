#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4286;

/* @source 0x801F2C60
 * @behavior Loads the shared gate byte D_801490C7; while that byte is nonzero
 *           the handler index byte at 0x801D4286 is set to 3. The shared status
 *           halfword D_801490A8 is then stored with 0xFFFF on every path, so
 *           that store runs once, after the branch joins. Takes no arguments
 *           and returns nothing; the 0x3 constant is materialised in $v0 and
 *           fills the branch delay slot.
 *           Shape: the 48 bytes at 0x801F2C60 are byte-identical (payload cmp,
 *           equal from the first word to the `jr $ra` nop) to the four
 *           recorded-exact siblings setStatusFFFFAndHandlerIndex3WhenGate at
 *           world01/area068/13@0x801F3930, world00/area003/13@0x801F2C48,
 *           world01/area041/13@0x801F3CD4 and world01/area061/13@0x801F2C50,
 *           which supply this body's statement order, operand forms and
 *           declaration style. The handler index 5 twin at
 *           world01/area059/13@0x801F2C78 differs in that one stored immediate
 *           alone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndHandlerIndex3WhenGateWorld01Area05013_801F2C60(void) {
  if (D_801490C7 != 0) {
    D_801D4286 = 3;
  }
  D_801490A8 = 0xFFFF;
}

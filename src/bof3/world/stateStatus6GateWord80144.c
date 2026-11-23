#include "bof3/bof3.h"

/* @source 0x801490C7 @kind data */
extern s8 D_801490C7;
/* @source 0x801490A8 @kind data */
extern u16 D_801490A8;
/* @source 0x801448ED @kind data */
extern u8 D_801448ED;
/* @source 0x80144F50 @kind data */
extern u32 D_80144F50;
/* @source 0x80148208 @kind data */
extern u8 *D_80148208;

/* @source 0x801F3AC8
 * @behavior Reads the shared gate byte D_801490C7. When it is zero, stores
 * 0x6F into the shared status halfword D_801490A8 and clears the shared byte
 * D_801448ED. Otherwise it compares the shared word D_80144F50 with an
 * unsigned bound of 0x1F4 (`lw`, `sltiu`): below the bound it stores 0x7C into
 * D_801490A8 and 0xFF into byte 0x7A of the pointer held at D_80148208, and at
 * or above the bound it stores 0x7D into D_801490A8 and 1 into D_801448ED.
 * Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStatus6GateWord80144(void) {
  if (D_801490C7 == 0) {
    D_801490A8 = 0x6F;
    D_801448ED = 0;
  } else if (D_80144F50 >= 0x1F4u) {
    D_801490A8 = 0x7D;
    D_801448ED = 1;
  } else {
    D_801490A8 = 0x7C;
    D_80148208[0x7A] = 0xFF;
  }
}

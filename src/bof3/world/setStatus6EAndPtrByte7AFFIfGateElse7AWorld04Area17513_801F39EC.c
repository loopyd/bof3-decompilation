#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 *D_80148208;

/* @source 0x801F39EC
 * @behavior Reads the shared gate byte D_801490C7. When it is zero, stores
 * 0x7A into the shared status halfword D_801490A8. Otherwise stores 0x6E
 * into that halfword and 0xFF into byte 0x7A of the pointer held at D_80148208.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus6EAndPtrByte7AFFIfGateElse7AWorld04Area17513_801F39EC(void) {
  if (D_801490C7 == 0) {
    D_801490A8 = 0x7A;
  } else {
    D_801490A8 = 0x6E;
    D_80148208[0x7A] = 0xFF;
  }
}

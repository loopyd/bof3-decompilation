#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 *D_80148208;

/* @source 0x801F38E8
 * @behavior Loads the shared gate byte D_801490C7 and, while it equals 1,
 * stores 0x5A into the shared status halfword D_801490A8 and 0xFF into the byte
 * at offset 0x7A of the pointer held at 0x80148208; when the gate byte differs
 * from 1 it stores 0xFFFF into D_801490A8 instead and touches nothing else.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus5AAndPtrByte7AFFIfGateElseFFFFWorld04Area17513_801F38E8(void) {
  if (D_801490C7 == 1) {
    D_801490A8 = 0x5A;
    D_80148208[0x7A] = 0xFF;
  } else {
    D_801490A8 = 0xFFFF;
  }
}

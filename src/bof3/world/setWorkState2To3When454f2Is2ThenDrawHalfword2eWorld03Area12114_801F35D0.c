#include "bof3/bof3.h"

/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;

void func_801F3B18(s16 arg0, s16 arg1);

/* @source 0x801F35D0
 * @behavior When the shared byte at 0x801454F2 is 2, writes 3 to byte 0x02 of
 * the work record published at the scratchpad pointer slot 0x44. Then reloads
 * that record and calls the local 0x801F3B18 drawer with 0x10 and the signed
 * halfword at record offset 0x2E. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkState2To3When454f2Is2ThenDrawHalfword2eWorld03Area12114_801F35D0(void) {
  u8* record;

  if (D_801454F2 == 2) {
    SPAD_PTR_SLOT(u8, 0x44)[2] = 3;
  }
  record = SPAD_PTR_SLOT(u8, 0x44);
  func_801F3B18(0x10, FIELD_REF(s16, record, 0x2E));
}

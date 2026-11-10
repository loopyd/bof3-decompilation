#include "bof3/bof3.h"

/* @source 0x8015C088 @kind function */
extern void func_8015C088(void);
/* @source 0x801490C7 @kind data */
extern s8 D_801490C7;
/* @source 0x801490A8 @kind data */
extern u16 D_801490A8;
/* @source 0x80146874 @kind data */
extern u8 D_80146874;
/* @source 0x80146875 @kind data */
extern u8 D_80146875;
/* @source 0x80148208 @kind data */
extern u8 *D_80148208;

/* @source 0x801F3AF4
 * @behavior When the signed shared gate byte is nonzero, stores status 0x4C.
 *           Otherwise calls the shared startup helper at 0x8015C088 and
 *           requests primary state 8. If the unsigned status halfword is
 *           0x44, requests substate 8, stores status 0x4B and increments
 *           the work record's unsigned halfword at offset 0x7E. Otherwise
 *           requests substate 0xA and stores status 0xFFFF. Takes no
 *           arguments and returns nothing.
 *
 * Name evidence: the original lb, lhu, sb and sh accesses establish the
 * gate, status constants and state-byte stores. The adjacent state setters
 * identify the shared gate/status cells; the area120 handler at 0x801F4350
 * supplies the pointer-cell and in-place work-halfword spelling. A table
 * word at 0x801F529C references this callback; it gives no argument roles.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectStatus4COr4BOrFFFFAndState8WhenGateClearWorld03Area14313_801F3AF4(void) {
  u8 *record;

  if (D_801490C7 == 0) {
    func_8015C088();
    D_80146874 = 8;
    if (D_801490A8 == 0x44) {
      record = D_80148208;
      D_80146875 = 8;
      D_801490A8 = 0x4B;
      *(u16 *)(record + 0x7E) += 1;
    } else {
      D_80146875 = 0xA;
      D_801490A8 = 0xFFFF;
    }
  } else {
    D_801490A8 = 0x4C;
  }
}

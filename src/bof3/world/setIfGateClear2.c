#include "bof3/bof3.h"

/* @source 0x801490C7 @kind unknown */
extern s8 D_801490C7;
/* @source 0x801448EC @kind unknown */
extern u8 D_801448EC;
/* @source 0x801490A8 @kind unknown */
extern u16 D_801490A8;

/* @source 0x801F2C04
 * @behavior Reads the shared gate byte D_801490C7 with a signed byte load and,
 *           when that byte is zero, writes 2 to the shared mode byte
 *           D_801448EC before storing 0xFFFF into the shared status halfword
 *           D_801490A8. The status store sits on both arms, so the halfword is
 *           written on every path and the gate byte only decides the mode
 *           write. Takes no arguments and returns nothing; the body is a leaf
 *           with no frame and no call, and the 0xFFFF constant is materialised
 *           in the branch delay slot and again after the mode store.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The 52 bytes at payload offset 4 compared equal (payload cmp, first word
 * through the `jr $ra` nop) to the spans at 0x801F2C04 of emi/world00/area016/13,
 * emi/world00/area033/13, emi/world01/area045/13 and emi/world02/area087/13 and
 * at 0x801F3764 of emi/world02/area088/13 and 0x801F2C04 of
 * emi/world03/area151/13, which had already carried this body as an exact lift.
 * Those rows supplied the name, the two-arm statement order and the declaration
 * style; the three shared cells and the 2/0xFFFF literals come from this
 * payload's own bytes. The cells sit outside this payload, so the shared main
 * executable defines them.
 */
void setIfGateClear(void) {
  if (D_801490C7 == 0) {
    D_801448EC = 2;
    D_801490A8 = 0xFFFFu;
  } else {
    D_801490A8 = 0xFFFFu;
  }
}

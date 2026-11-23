#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u8 D_801448EC;
extern u16 D_801490A8;

/* @source 0x801F2C04
 * @behavior Reads the shared gate byte 0x801490C7 and, when that byte is zero,
 *           writes 2 to the shared mode byte 0x801448EC and then stores 0xFFFF
 *           into the shared status halfword 0x801490A8. The status store sits on
 *           both arms, so the halfword is written on every path. Takes no
 *           arguments and returns nothing: the body is a leaf with no frame, the
 *           0xFFFF constant is materialised in the `bnez` delay slot and again
 *           after the mode store, and each cell address is rebuilt by its own
 *           `lui` because the byte store needs $at.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Clone: this 0x34-byte span at payload offset 4 is byte-identical (52/52 bytes
 * by `cmp` of the extracted entries) to the lifted
 * setStatusFFFFAndSelectMode2IfGateClearWorld03Area15113_801F2C04, whose splat
 * row is `c`, and to the same body in emi/world00/area016/13,
 * emi/world00/area033/13 and emi/world02/area087/13. That twin supplied the
 * two-arm statement order, the declaration style and the name. All three cells
 * are main-executable addresses below this payload's load address 0x801F2C00,
 * so the twin's bindings hold here. 0x801448EC is the byte those family
 * handlers write beside the 0x801490A8 status halfword; the 0x2 and 0xFFFF
 * literals come from this span's own bytes. The payload names this body only
 * from the table word at 0x801F4E48 and no `jal` reaches it, so its argument
 * roles stay structural.
 */
void stateStatusSelectGate(void) {
  if (D_801490C7 == 0) {
    D_801448EC = 2;
    D_801490A8 = 0xFFFFu;
  } else {
    D_801490A8 = 0xFFFFu;
  }
}

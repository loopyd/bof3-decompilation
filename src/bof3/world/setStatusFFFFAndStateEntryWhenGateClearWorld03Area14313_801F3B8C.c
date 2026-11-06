#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801448EB;
extern u8 D_801448EC;
extern u8 D_801448ED;

/* @source 0x801F3B8C
 * @behavior Overlay state setter: stores 0xFFFF into the shared status halfword
 *           at 0x801490A8 on every path (`ori v0,0,0xFFFF`, `sh`), and when the
 *           shared gate byte at 0x801490C7 is zero (`lb`, so the cell is s8) it
 *           writes the shared state-entry tuple 0x801448EB = 0x0A, 0x801448EC =
 *           0 and 0x801448ED = 0xFF (`addiu v0,0,0xA` fills the `bnez` delay
 *           slot, then each `sb` carries its own constant in `v0`). No
 *           arguments, no return value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndStateEntryWhenGateClearWorld03Area14313_801F3B8C(void) {
  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 0) {
    D_801448EB = 0xA;
    D_801448EC = 0;
    D_801448ED = 0xFF;
  }
}

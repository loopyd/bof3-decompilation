#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern s8 D_801448EB;
extern s8 D_801448EC;
extern s8 D_801448ED;

/* @source 0x801F4320
 * @behavior Overlay state setter: stores 0xFFFF into the shared status halfword
 *           at 0x801490A8 on every path (`ori v0,0,0xFFFF`, `sh`), and when the
 *           shared gate byte at 0x801490C7 is zero (`lb`, so the cell is s8) it
 *           writes the shared state-entry tuple 0x801448EB = 0x0A, 0x801448EC =
 *           0 and 0x801448ED = 4 (`addiu v0,0,0xA` fills the `bnez` delay slot,
 *           then three `sb` with `$zero` for the middle byte). No arguments, no
 *           return value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndStateEntryWhenGateClearWorld02Area10814_801F4320(void) {
  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 0) {
    D_801448EB = 0xA;
    D_801448EC = 0;
    D_801448ED = 4;
  }
}

#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801448EB;
extern u8 D_801448EC;
extern u8 D_801448ED;

/* @source 0x801F2C04
 * @behavior Overlay state setter: stores 0xFFFF into the shared status halfword
 *           at 0x801490A8 on every path (`ori v0,0,0xFFFF`, `sh`), and when the
 *           shared gate byte at 0x801490C7 is zero (`lb`, so the cell is s8) it
 *           writes the shared state-entry tuple 0x801448EB = 0x0A, 0x801448EC =
 *           5 and 0x801448ED = 0xFF (`addiu v0,0,0xA` fills the `bnez` delay
 *           slot, then each `sb` carries its own constant in `v0`). No
 *           arguments, no return value; the body is a leaf with no frame.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The 0x44-byte span was proven byte-identical (68/68 bytes, payload `find`
 * over the extracted overlays) to the lifted-exact World01Area04013 body at
 * 0x801F3074, which supplied the statement order and declaration style, and to
 * six further extracted spans: world00/area001 0x801F2C04, world00/area032
 * 0x801F36A0, world01/area041 0x801F3DBC, world03/area117 0x801F40CC,
 * world03/area141 0x801F3AC0 and world03/area143 0x801F3BCC. All five cells sit
 * in the main executable outside every payload, so their names carry over
 * unchanged. The one statement that parts this body from its same-target
 * neighbour at 0x801F2C48 is the 5 stored at 0x801448EC, where that neighbour
 * stores zero.
 */
void setStatusFFFFAndStateEntryProgress5WhenGateClearWorld00Area02313_801F2C04(void) {
  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 0) {
    D_801448EB = 0xA;
    D_801448EC = 5;
    D_801448ED = 0xFF;
  }
}

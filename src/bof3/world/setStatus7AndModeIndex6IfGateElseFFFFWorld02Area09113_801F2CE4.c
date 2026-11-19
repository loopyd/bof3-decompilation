#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F2CE4
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read
 *           (`lb`, so the cell is s8) and, while that byte is nonzero, stores 7
 *           into the shared status halfword D_801490A8 and then 6 into the
 *           shared mode byte at 0x801D4285. When the gate byte is zero it
 *           stores 0xFFFF into D_801490A8 instead and writes nothing else.
 *           Takes no arguments and returns nothing; leaf with no frame, and the
 *           7 is materialised in $v0 and fills the branch delay slot.
 *           Shape: the 68 bytes 0x801F2CE4..0x801F2D28 differ from the recorded
 *           exact sibling setStatus24AndModeIndex6IfGateElseFFFFWorld01Area05513_801F2CE8
 *           in exactly two words - this span's stored status immediate 7
 *           against that sibling's 0x24, and the internal `j` immediate at
 *           0x801F2D10, whose value moves by 0x1C, the delta between the two
 *           spans - so every relative branch, other literal and cell reference
 *           is identical. The payload's own next run entry at 0x801F2D28 holds
 *           the same body again, differing in that `j` immediate alone (0x44
 *           higher, the span delta); that row was still an asm row when this
 *           comparison ran, so its bytes are original evidence.
 *           Boundary and role: the only reference to this address is the
 *           address-taken word at 0x801F3B3C of the ordered handler run
 *           0x801F2C04, 0x801F2C84, 0x801F2CB4, 0x801F2CE4, 0x801F2D28
 *           (payload offset 0xF3C, the sole absolute word naming this address),
 *           with no `jal` immediate anywhere in the payload, so its role stays
 *           address-taken and its parameter list empty. The body ends with its
 *           own `jr $ra` and the 0x44-byte span ends exactly at the next run
 *           entry 0x801F2D28.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus7AndModeIndex6IfGateElseFFFFWorld02Area09113_801F2CE4(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 7;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}

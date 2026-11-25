#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F2D28
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read
 *           (`lb`, so the cell is s8) and, while that byte is nonzero, stores 7
 *           into the shared status halfword D_801490A8 and then 6 into the
 *           shared mode byte at 0x801D4285. When the gate byte is zero it
 *           stores 0xFFFF into D_801490A8 instead and writes nothing else.
 *           Takes no arguments and returns nothing; leaf with no frame, and the
 *           7 is materialised in $v0 to fill the branch delay slot.
 *           Shape: the 68 bytes here are the same body as the recorded exact
 *           sibling stateStatus7ModeGate,
 *           0x44 bytes earlier: a byte comparison of the two payload spans
 *           differs in exactly one word, the internal `j` immediate at
 *           0x801F2D50, whose value moves by 0x44 - the delta between the two
 *           spans - so every relative branch, literal and cell reference is
 *           identical. The statement order below is that sibling's; the else
 *           arm is reached by the same `beqz` on the loaded gate byte.
 *           Boundary and role: the only reference to this address is the
 *           address-taken word at 0x801F3B40 (payload offset 0xF40) of the
 *           ordered handler run 0x801F2C04, 0x801F2C84, 0x801F2CB4, 0x801F2CE4,
 *           0x801F2D28, with no `jal` immediate anywhere in the payload, so the
 *           role stays address-taken and the parameter list empty. The body
 *           ends with its own `jr $ra` and the 0x44-byte span ends exactly at
 *           the next subsegment entry 0x801F2D6C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStatusSevenGate(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 7;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}

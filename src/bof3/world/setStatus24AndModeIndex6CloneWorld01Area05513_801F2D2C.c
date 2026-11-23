#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F2D2C
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read and,
 *           while that byte is nonzero, stores 0x24 into the shared status
 *           halfword D_801490A8 and then 6 into the shared mode byte at
 *           0x801D4285. When the gate byte is zero it stores 0xFFFF into
 *           D_801490A8 instead and writes nothing else. Takes no arguments and
 *           returns nothing; leaf with no frame, and the 0x24 constant is
 *           materialised in $v0 and fills the branch delay slot. Its only
 *           reference is the fifth word of the pointer table based at
 *           0x801F3FB4, whose leading entries 0x801F2C04, 0x801F2C88, 0x801F2CB8,
 *           0x801F2CE8 and 0x801F2D2C are entry points of this payload (no `jal`
 *           reaches this address).
 *           Shape: the 68 bytes at 0x801F2D2C differ from the recorded-exact
 *           sibling stateStatus24ModeGate
 *           in exactly one word, the internal `j` immediate at 0x801F2D54, whose
 *           value moves by 0x44 bytes, the delta between the two spans, so every
 *           relative branch, literal and cell reference is identical and the two
 *           spans are one body (payload `cmp`). The stored status immediate 0x24
 *           and mode index 6 are this span's own, and the sibling supplies the
 *           statement order and declaration style.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus24AndModeIndex6CloneWorld01Area05513_801F2D2C(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 0x24;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}

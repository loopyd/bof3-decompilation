#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F2CE8
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read and,
 *           while that byte is nonzero, stores 0x24 into the shared status
 *           halfword D_801490A8 and then 6 into the shared mode byte at
 *           0x801D4285. When the gate byte is zero it stores 0xFFFF into
 *           D_801490A8 instead and writes nothing else. Takes no arguments and
 *           returns nothing; leaf with no frame, and the 0x24 constant is
 *           materialised in $v0 and fills the branch delay slot.
 *           Shape: the 68 bytes 0x801F2CE8..0x801F2D2C differ from the
 *           recorded-exact sibling
 *           setStatus2FAndModeIndex6IfGateElseFFFFWorld02Area09813_801F5B0C in
 *           exactly two words - the arm's stored status immediate, 0x24 in this
 *           span, and the internal `j` immediate at 0x801F2D10, whose value
 *           moves by 0x2E24 bytes, the delta between the two spans - so every
 *           relative branch, other literal and cell reference is identical and
 *           the two spans are one body (payload `cmp`). The byte-identical twin
 *           inside this payload at 0x801F2D2C differs in that `j` immediate
 *           alone, and a masked scan of all 398 extracted payloads found no
 *           other member. The handler role and the argument-free signature come
 *           from the pointer table at 0x801F3FB4, whose entries 0x801F2C04,
 *           0x801F2C88, 0x801F2CB8, 0x801F2CE8, 0x801F2D2C are the only
 *           reference to this address (no `jal`).
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStatus24ModeGate(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 0x24;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}

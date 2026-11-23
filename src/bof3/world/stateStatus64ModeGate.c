#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F3D34
 * @behavior Loads the shared gate byte D_801490C7 and, while that byte is
 *           nonzero, stores 0x64 into the shared status halfword D_801490A8 and
 *           6 into the shared mode index byte at 0x801D4285; when the gate byte
 *           is zero it stores 0xFFFF into D_801490A8 instead and touches
 *           nothing else. Takes no arguments and returns nothing; leaf with no
 *           frame, and the 0x64 constant is materialised in $v0 and fills the
 *           branch delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Naming evidence: the .text body is the same 0x44-byte body as the exact
 * sibling setStatus64AndModeIndex6IfGateElseFFFFWorld01Area04113_801F3D78.
 * One cmp of the two payload spans (offsets 4404 and 4472, 0x44 bytes each)
 * shows every word equal except the absolute `j` word, whose value differs by
 * exactly the 0x44 span delta; the relative branches, the literal 0x64, the
 * `ori 0xFFFF` arm and the tail all stay identical, so the two spans are one
 * body and the description above is this function's own constants and cells.
 * Both arms read the same shared gate byte D_801490C7 and write the same
 * shared status halfword D_801490A8, matching the sibling grammar for this
 * overlay, and the span has its own leading `lui`/`lb` and its own `jr $ra`
 * before the next boundary at 0x801F3D78, so it is a whole leaf function.
 * The overlay's handler pointer run at 0x801F5614 lists this address beside
 * the neighbouring status handlers, which proves residency as a handler but
 * names no further role, so the store roles stay the evidence for the name.
 */
void stateStatus64ModeGate(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 0x64;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}

#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F376C
 * @behavior Loads the shared gate byte D_801490C7 and, while that byte is
 * nonzero, stores 0x9C into the shared status halfword D_801490A8 and 6 into the
 * shared mode byte at 0x801D4285; when the gate byte is zero it stores 0xFFFF
 * into D_801490A8 instead and touches nothing else. Takes no arguments and
 * returns nothing; leaf with no frame, and the 0x9C constant is materialised in
 * $v0 and fills the branch delay slot. No `jal` in this payload reaches this
 * address, and its only in-payload reference is the little-endian handler word
 * at 0x801F4A44 inside the script/data blob D_801F3B34, so the entry is a
 * script handler reached only through that table and the blob carries no
 * prototype, which keeps the argument roles structural.
 * Shape: the 68 bytes at 0x801F376C were compared word by word with the lifted
 * sibling setStatus88AndModeIndex6IfGateElseFFFFWorld00Area03713_801F36E4 and
 * differed in two words only - the status literal stored on the gated arm (0x88
 * there, 0x9C here) and the internal `j` immediate, which moved by 0x88 bytes,
 * exactly the delta between the two spans - so every relative branch, cell
 * reference and store site was identical and the two spans were one body. That
 * sibling supplied the statement order, operand forms and declaration style;
 * this span's own bytes supplied its status literal and its `j`. One payload
 * scan for the 68-byte span over every extracted overlay found it once only, at
 * its own offset, so no byte-identical twin existed to clone and that lever was
 * declined.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus9CAndModeIndex6IfGateElseFFFFWorld00Area03713_801F376C(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 0x9C;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}

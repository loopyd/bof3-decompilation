#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F37B0
 * @behavior Loads the shared gate byte D_801490C7 and, while that byte is
 * nonzero, stores 0x9C into the shared status halfword D_801490A8 and 6 into the
 * shared mode byte at 0x801D4285; when the gate byte is zero it stores 0xFFFF
 * into D_801490A8 instead and touches nothing else. Takes no arguments and
 * returns nothing; leaf with no frame, and the 0x9C constant is materialised in
 * $v0 and fills the branch delay slot. No `jal` in this payload reaches this
 * address, and its only in-payload reference is the little-endian handler word
 * at 0x801F4A48 inside the script/data blob D_801F3B34, so the entry is a
 * script handler reached only through that table and the blob carries no
 * prototype, which keeps the argument roles structural.
 * Shape: the 68 bytes at 0x801F37B0 were compared byte for byte with the
 * recorded-exact sibling stateStatus9ModeGate
 * and differed in one byte only, the low byte of the internal `j` immediate at
 * 0x801F37D8, whose branch target moves by 0x44 bytes, exactly the delta between
 * the two spans - every relative branch, literal, cell reference and store site
 * was identical, so the two spans were one body and the sibling supplied the
 * statement order, operand forms and declaration style. That single relocated
 * word also declined the clone adapter, which compares whole reviewed ranges, so
 * the body was transcribed from the sibling with this span's own address; no
 * other lever applied and no payload scan was needed for a byte-identical twin.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateModeGateClone(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 0x9C;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}

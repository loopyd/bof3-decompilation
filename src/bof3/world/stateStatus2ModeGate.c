#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F5AC8
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read
 *           (`lb`, so the cell is s8). While that byte is nonzero it stores the
 *           status halfword 0x2F into D_801490A8 (`addiu v0,0,0x2F` in the
 *           branch delay slot, `sh`) and then 6 into the shared mode index byte
 *           at 0x801D4285 (`addiu v0,0,6`, `sb`), and it leaves through the
 *           shared tail without touching that halfword again; when the gate byte
 *           is zero it stores 0xFFFF into D_801490A8 instead (`ori
 *           v0,0,0xFFFF`, `sh`) and writes no mode byte. Takes no arguments,
 *           returns nothing and is a leaf with no frame.
 *           Constant/variance evidence: the 68 bytes 0x801F5AC8..0x801F5B0C
 *           differ from the exact lift
 *           setStatus54AndModeIndex6IfGateElseFFFFWorld03Area14313_801F3A44
 *           only in the stored status literal (0x2F here, 0x54 there) and in the
 *           `j` target word, whose address shifts by exactly the 0x2084 delta
 *           between the two spans; a full payload scan of out/binaries found no
 *           byte-identical copy of this span. The 0x801D4285 mode index byte is
 *           the cell that family and the sibling lifts in world01/area050/13
 *           name the same way, and the 0x801490C7 gate byte with the 0x801490A8
 *           status halfword are the shared pair the two exact siblings in this
 *           payload read and write. Inside this payload the function is entry 2
 *           of the word handler table at 0x801F64B8, whose own word at
 *           0x801F64C0 is the only reference to this address (no `jal`); the
 *           neighbouring entries are the two exact siblings at 0x801F5A68 and
 *           0x801F5A98, so the handler role and the argument-free signature come
 *           from the table rather than from a caller.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStatus2ModeGate(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 0x2F;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}

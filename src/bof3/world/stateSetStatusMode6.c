#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F5B0C
 * @behavior Overlay helper that sets the shared status word from the shared
 *           gate byte. It loads the gate byte at 0x801490C7 with a signed byte
 *           read (`lb`, so the cell is s8) and, while that byte is nonzero,
 *           stores the status halfword 0x2F into D_801490A8 (`addiu v0,0,0x2F`
 *           in the branch delay slot, `sh`) and then 6 into the shared mode
 *           index byte at 0x801D4285 (`addiu v0,0,6`, `sb`) before leaving
 *           through the shared tail; when the gate byte is zero it stores
 *           0xFFFF into D_801490A8 instead (`ori v0,0,0xFFFF`, `sh`) and writes
 *           no mode byte. No arguments, no return value, leaf with no frame.
 *           Constant/variance evidence: the 68 bytes 0x801F5B0C..0x801F5B50
 *           differ from the exact sibling
 *           stateStatus2ModeGate in
 *           exactly one word, the `j` target at 0x801F5B34, whose immediate
 *           moves from 0x0807D6C1 to 0x0807D6D2 - 0x11 words, the 0x44-byte
 *           delta between the two spans - so every relative branch, literal and
 *           cell reference is identical and the two spans are one body
 *           (payload `cmp` of the two spans). That equality, not a shared
 *           claim, is the naming evidence: the stored status 0x2F and mode
 *           index 6 are this span's own bytes. Inside this payload the function
 *           is entry 3
 *           of the word handler table at 0x801F64B8, whose word at 0x801F64C4
 *           is the only reference to this address (no `jal`); entries 0..2 of
 *           that table are the exact lifts at 0x801F5A68, 0x801F5A98 and
 *           0x801F5AC8, so the handler role and the argument-free signature come
 *           from the table rather than from a caller.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateSetStatusMode6(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 0x2F;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}

#include "bof3/bof3.h"

extern u8 D_80143F06;
extern u8 D_80143F03;
extern u8 D_80147ACF;

/* @source 0x801F33BC
 * @behavior Overlay front-side selector, two independent arms, each a
 * conjunction of the two shared front-side bytes. It reads the staged byte
 * 0x80143F06 and the live byte 0x80143F03 and writes one of two constants into
 * the shared byte 0x80147ACF: pending 1 with live 4 stores 0x02, pending 4 with
 * live 1 stores 0x04. Every other pair leaves 0x80147ACF alone. The first arm
 * reuses the constant 4 materialised in its entry branch delay slot for the
 * live-byte test, and the second arm reloads 0x80143F06 before comparing it
 * against 4, so each arm loads both cells after the staged-byte decision.
 * Takes no arguments, calls nothing, returns nothing; leaf with no stack frame
 * and one exit at 0x801F341C.
 *
 * Evidence: 0x80143F06 is the copy of 0x80143F03 that the front-entry staging
 * routine at 0x8019FAA0 publishes at its 0x8019FB48 store, so the pair is the
 * staged and the live front-side reading. Seven shipped payloads (ten store
 * sites) write 0x80147ACF and none loads it, so its engine role stays unknown
 * and the name describes the condition and the store.
 *
 * Shape evidence: selectCrossedFrontSideStateWorld04Area18813_801F4D88
 * (emi/world04/area188/13 0x801F4D88) is the same 26-instruction body over the
 * same three cells, with the identical branch/load/constant order and its own
 * literals (0x0C/0x0E). No byte-identical sibling exists: the 0x68-byte range
 * matches no other payload in out/binaries. The payload holds no `jal` to
 * 0x801F33BC: its one word reference is at 0x801F43F4 in the tail blob, so the
 * dispatch index that selects it is unevidenced.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectCrossedFrontSideState2Or4World03Area13613_801F33BC(void) {
  if (D_80143F06 == 1) {
    if (D_80143F03 == 4) {
      D_80147ACF = 0x02;
    }
  }
  if (D_80143F06 == 4) {
    if (D_80143F03 == 1) {
      D_80147ACF = 0x04;
    }
  }
}

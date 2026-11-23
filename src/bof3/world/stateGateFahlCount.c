#include "bof3/bof3.h"

extern s8 D_801490C7;  /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
/* @source 0x801454F3 @kind data — Fahl battle-encounter counter, an unsigned
 * byte whose recruit gate is `< 30` (docs/specs/bof3-eu/22-further-address-findings.md);
 * this body reads it with `lbu` and tests it with `sltiu ..., 0x1E`. */
extern u8 D_801454F3;
/* @source 0x801D4285 @kind bss — shared mode index: selects the handler from the
 * D_801D41E0 table and is set to 6 after an entry action starts
 * (include/bof3/ui/sisyou00_internal.h). */
extern u8 D_801D4285;

/* @source 0x801F2C04
 * @behavior Loads the shared gate byte D_801490C7; while that byte is nonzero
 *           the shared status halfword D_801490A8 is stored with 0x84. When the
 *           gate byte is zero it loads the Fahl battle counter D_801454F3 and,
 *           while that counter is below 30, stores 0x83 into D_801490A8; a
 *           counter of 30 or more stores 0x82 into D_801490A8 instead and
 *           touches nothing else. The two 0x84/0x83 paths join and then store 6
 *           into the shared mode index byte D_801D4285, so that store runs once
 *           after the branch join; the 0x82 path leaves the mode index alone.
 *           Takes no arguments and returns nothing; leaf with no frame and no
 *           call, each status constant is materialised in $v0 and fills the
 *           branch delay slot of the test that selects it, and the joined store
 *           reads that same $v0.
 *           Name evidence: 0x801490C7/0x801490A8/0x801D4285 are the shared gate
 *           byte, status halfword and mode index this target's own exact lifts
 *           (setStatusFFFFAndHandlerIndex3WhenGateWorld01Area05013_801F2C60 and
 *           stateStatus84ModeSix)
 *           already read and write at those addresses. 0x801454F3 is the Fahl
 *           battle-encounter counter that the shipped EU address register ties
 *           to this overlay's master-recruitment gate checks, and its `< 30`
 *           bound is this span's own 0x1E immediate. The three stored literals
 *           0x84/0x83/0x82 and the mode index 6 come from this span's bytes; the
 *           payload names this body from no table word and no `jal` reaches it,
 *           so its argument roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateGateFahlCount(
    void) {
  u16 status;

  if (D_801490C7 != 0) {
    status = 0x84;
  } else if (D_801454F3 < 0x1E) {
    status = 0x83;
  } else {
    D_801490A8 = 0x82;
    return;
  }
  D_801490A8 = status;
  D_801D4285 = 6;
}

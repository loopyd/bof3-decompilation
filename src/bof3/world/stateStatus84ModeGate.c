#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
/* @source 0x801D4285 @kind bss — shared mode index: selects the handler from
 * the D_801D41E0 table and is set to 6 when an entry action starts
 * (include/bof3/ui/sisyou00_internal.h). */
extern u8 D_801D4285;

/* @source 0x801F3990
 * @behavior Overlay entry handler that publishes the shared status halfword
 *           D_801490A8 and the shared mode index byte D_801D4285: while the
 *           gate byte D_801490C7 is nonzero it stores 0x84 into the status and
 *           6 into the mode index; when the gate byte is zero it stores 0xFFFF
 *           into the status instead and touches nothing else. Takes no
 *           arguments and returns nothing; leaf with no frame. The 0x84
 *           constant is materialised in $v0 and fills the branch delay slot of
 *           the gate test, and each arm stores the status halfword at its own
 *           site.
 *           Constant/variance evidence: the 68 bytes 0x801F3990..0x801F39D4
 *           differ from the sibling at emi/world01/area050/13@0x801F2D04 —
 *           whose own lift had measured exact when this seed was written — only
 *           in the immediate of this span's own `j`, which shifts by the exact
 *           0xC8C span delta (0x801F2D40 in the sibling, 0x801F39CC here), so
 *           every literal and every cell binding above comes from these
 *           original bytes. The adjacent span 0x801F39D4 carries the same body
 *           again.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStatus84ModeGate(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 0x84;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}

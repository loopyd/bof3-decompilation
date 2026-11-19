#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 g_ScenarioProgress;

/* @source 0x801F3CE4
 * @behavior Overlay handler word at 0x801F4914 of the local pointer run based
 *           at 0x801F48F4: loads the shared gate byte D_801490C7 with a signed
 *           byte read (`lb`, so the cell is s8), stores 0xFFFF into the shared
 *           status halfword D_801490A8 (`ori v0,0,0xFFFF`, `sh`), then stores
 *           0x23 into the shared scenario progress byte g_ScenarioProgress
 *           while the gate byte is nonzero and 0x1E when it is zero. The
 *           progress byte is a conditional expression written straight into
 *           the cell, which is why the gate load stays in v1, the selected
 *           constant lands in v0 from the branch delay slot, and the cell
 *           takes one `sb`. Takes no arguments, returns nothing and allocates
 *           no frame.
 *
 * Name evidence: the two stored values are scenario progress stages of the
 * shared progress byte 0x80146864, which
 * src/bof3/scenario/runSecondaryStateSequenceForPrimaryState16ScenarioScena0600_801FC498.c
 * advances through 0x1E and 0x23 in the same cell. The shape is the exact
 * sibling setStatusFFFFAndScenarioProgress14IfGateElse0AWorld03Area12013_801F42F0,
 * whose 0x30-byte span under out/binaries differs from this one in exactly its
 * two stored literals (0x14/0xA here 0x23/0x1E) and names the same shared
 * cells; a scan of the 398 extracted payloads under out/binaries found no
 * byte-identical copy of this span, so the clone lever was declined and the
 * body was seeded from that sibling's statement order, operand forms and
 * declaration style with this span's own literals. The suffix carries the
 * in-payload address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndScenarioProgress23IfGateElse1EWorld03Area14413_801F3CE4(
    void) {
  s32 gate;

  gate = D_801490C7;
  D_801490A8 = 0xFFFFu;
  g_ScenarioProgress = (gate != 0) ? 0x23 : 0x1E;
}

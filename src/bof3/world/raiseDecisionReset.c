#include "bof3/bof3.h"

/* Shared overlay halfword at 0x801490A4, bound by the shared map. Its bit 0x2
 * is the decision flag the UI step handlers read (shop sub-step
 * redrawPanelStripThenConditionallyAdvanceUiSubstep takes its advance decision
 * from `D_801490A4 & 0x2`, setUiFlagsGated tests the same bit) and this overlay
 * step is the writer that raises it. The two accesses share one `$v0` base that
 * the original materialises with `lui`/`addiu`. */
extern u16 D_801490A4; /* @source 0x801490A4 @kind data */

/* Shared scenario progress byte at 0x80146864, bound by the shared map. The
 * store re-materialises `$at` for its own `lui`/`sb` pair. */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind data */

/* Shared work-area reset at 0x80196070 (clearWorkFlags in emi/etc/game/00). It
 * sits outside this overlay's payload, so only its prototype is declared. It
 * takes no arguments, returns nothing, and zeroes the scratchpad work-area
 * header bytes 0x00-0x04, which puts the work object's handler index back to
 * zero. */
void func_80196070(void);

/* @source 0x801F318C
 * @behavior Work-record state handler of this overlay: word 7 of the state-byte
 *           handler table D_801F4624, whose own word at 0x801F4640 holds this
 *           address and which the overlay dispatcher
 *           callStateTable4624World01 indexes with
 *           byte +1 of the record published at the scratchpad pointer cell
 *           0x1F800044. It reads the shared halfword D_801490A4 with `lhu`,
 *           writes 0x32 into the shared scenario progress byte
 *           g_ScenarioProgress, ORs bit 0x2 into the halfword it read and
 *           stores it back through the same base, then calls the shared
 *           work-area reset func_80196070, whose `sh` fills the call's delay
 *           slot. The raised bit is the shared decision flag the UI step
 *           handlers test, so this step requests that decision at scenario
 *           progress 50 and then clears the work area header. The 0x18-byte
 *           frame keeps `$ra` at +0x10 and the trailing slot stays `nop`, so the
 *           body takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseDecisionReset(
    void) {
  u16* flags;
  u16 value;

  flags = &D_801490A4;
  value = *flags;
  g_ScenarioProgress = 0x32;
  value |= 2;
  *flags = value;
  func_80196070();
}

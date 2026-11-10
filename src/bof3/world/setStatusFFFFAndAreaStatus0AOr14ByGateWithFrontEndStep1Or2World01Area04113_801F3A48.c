#include "bof3/bof3.h"

/* Shared gate byte this overlay's entry handlers branch on. It is read with lb,
 * so it is declared signed here.
 * @source 0x801490C7 @kind unknown */
extern s8 D_801490C7;

/* Shared status halfword the entry handlers latch, stored with sh.
 * @source 0x801490A8 @kind unknown */
extern u16 D_801490A8;

/* Shared area status byte the entry handlers seed, stored with one sb.
 * @source 0x80146865 @kind unknown */
extern u8 D_80146865;

/* Shared front-end step helper at 0x801BE1B0, selected by a step number. */
void func_801BE1B0(u32 step);

/* @source 0x801F3A48
 * @behavior Overlay entry handler word at 0x801F55F8 of the local pointer run
 *           based at 0x801F42C8: latches 0xFFFF into the shared status
 *           halfword 0x801490A8 (`ori v0,0,0xFFFF`, `sh`), then branches on
 *           the shared gate byte 0x801490C7 read with `lb`. While that byte
 *           reads zero it runs the shared front-end step helper 0x801BE1B0
 *           with step 1 and yields 0x0A; once the gate byte is set it runs the
 *           same helper with step 2 and yields 0x14. Both arms converge on one
 *           `sb` of that value to the shared area status byte 0x80146865, so
 *           the value is carried in `$v0` across the join. Takes no arguments,
 *           returns nothing and reserves a 0x18-byte frame for its two calls.
 *
 * Name evidence: the two arms differ only in the front-end step they pass and
 *   the area status byte they leave behind, so the name carries both pairings.
 *   0x80146865 is the shared area status byte: sibling
 *   startStateEntry0BProgress0AOr0WhenMapRangeAndScenarioGateWorld01Area04613_801F2CE4
 *   stores 1 into it beside the state-entry tuple and the scenario overlays
 *   clear it as a shared state byte, while this target's own gate handlers use
 *   0x801490C7/0x801490A8 for the same latch-then-branch shape. The suffix
 *   carries the in-payload address because no caller-visible role is evidenced.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndAreaStatus0AOr14ByGateWithFrontEndStep1Or2World01Area04113_801F3A48(
    void) {
  s32 areaStatus;

  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 0) {
    func_801BE1B0(1);
    areaStatus = 0xA;
  } else {
    func_801BE1B0(2);
    areaStatus = 0x14;
  }
  D_80146865 = areaStatus;
}

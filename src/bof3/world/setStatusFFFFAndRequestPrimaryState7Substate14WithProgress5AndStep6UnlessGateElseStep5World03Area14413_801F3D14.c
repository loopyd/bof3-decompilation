#include "bof3/bof3.h"

/* Shared gate byte this overlay's entry handlers branch on. It is read with lb,
 * so it is declared signed here. */
extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */

/* Shared status halfword the entry handlers latch, stored with sh. */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */

/* Shared scenario progress byte the first-visit arm latches. */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */

/* Shared primary state byte the first-visit arm requests, and the secondary
 * sub-state byte it seeds beside it. */
extern u8 D_80146874; /* @source 0x80146874 @kind unknown */
extern u8 D_80146875; /* @source 0x80146875 @kind unknown */

/* Shared front-end step helper at 0x801BE1B0, selected by a step number. */
void func_801BE1B0(u32 step);

/* @source 0x801F3D14
 * @behavior Overlay handler word at 0x801F4918 of the local pointer run based
 *           at 0x801F48F4: latches 0xFFFF into the shared status halfword
 *           0x801490A8, then branches on the shared gate byte 0x801490C7 read
 *           with `lb`. When that byte reads zero it stores 5 into the shared
 *           scenario progress byte 0x80146864, runs the shared front-end step
 *           helper 0x801BE1B0 with 6, requests primary state 7 and seeds
 *           secondary sub-state 0x14 in the shared cells 0x80146874 and
 *           0x80146875; when the gate byte is nonzero it runs only 0x801BE1B0
 *           with 5 and leaves the progress and state cells alone. Takes no
 *           arguments, returns nothing and reserves a 0x18-byte frame for its
 *           two calls.
 *
 * Name evidence: the body is the area144 gate-entry shape, one of the
 * `*World03Area14413_801F3xxx` handler words in the 0x801F48F4 run, and its two
 * arms differ only in the front-end step they pass (6 on the first visit, 5
 * once the gate byte is set). The cell roles come from the sibling sources that
 * name the same addresses: 0x80146874 is the primary state request cell
 * (`D_80146874 = 7` reads as requesting primary state 7 in
 * src/bof3/scenario/seedPrimaryStateOnRecordCallback_801FC818.c and the
 * `*AndRequestPrimaryState7*` scenario seeds), and 0x80146875 is the secondary
 * sub-state byte those seeds clear or set. The suffix carries the in-payload
 * address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndRequestPrimaryState7Substate14WithProgress5AndStep6UnlessGateElseStep5World03Area14413_801F3D14(
    void) {
  s32 gate;

  gate = D_801490C7;
  D_801490A8 = 0xFFFFu;
  if (gate == 0) {
    g_ScenarioProgress = 5;
    func_801BE1B0(6);
    D_80146874 = 7;
    D_80146875 = 0x14;
  } else {
    func_801BE1B0(5);
  }
}

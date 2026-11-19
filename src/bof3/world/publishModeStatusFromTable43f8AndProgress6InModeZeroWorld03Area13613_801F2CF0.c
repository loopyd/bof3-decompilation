#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u16 D_801F43F8[];
extern u8 g_ScenarioProgress;

/* @source 0x801F2CF0
 * @behavior Overlay mode handler: reads the shared mode byte 0x801490C7 once
 *           with `lui`/`lb` (signed byte) and keeps it in a register across the
 *           table load, indexes this payload's halfword table 0x801F43F8 by
 *           `sll` one and `addu` into $at, and publishes the fetched halfword
 *           into the shared status halfword 0x801490A8 with `sh`. It then tests
 *           the same byte with `bnez` and, only while that byte is zero, stores
 *           0x06 into the shared scenario progress byte 0x80146864; the branch
 *           delay slot carries the constant materialisation. No arguments, no
 *           calls, no other stores, leaf with no frame and one exit.
 *
 *           Evidence for the table: the 0x0A-byte span at file offset 0x17F8
 *           holds five halfwords, 0xFFFF, 0x000B, 0xFFFF, 0x000B and 0x4722, so
 *           the base is a target-local lookup table rather than code, and no
 *           source in the tree binds it.
 *
 *           Evidence for the entry: this body is entry 2 of the pointer run at
 *           0x801F4348 in this payload's tail blob, so the dispatch index that
 *           selects it is unevidenced and its argument roles stay structural.
 *           This 0x3C-byte span appears in no other payload under
 *           out/binaries, so no byte-identical sibling exists and the body is
 *           written from this span's own bytes. Shape only: the already-exact
 *           publishModeStatusFromTable362cWorld00Area01013_801F2C84 carries the
 *           same mode-load/table-index/status-store statement order, and
 *           func_801F3650 (emi/world03/area027/13) writes g_ScenarioProgress
 *           from the same mode byte, which this function narrows to the single
 *           mode-zero arm.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusFromTable43f8AndProgress6InModeZeroWorld03Area13613_801F2CF0(
    void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F43F8[mode];
  if (mode == 0) {
    g_ScenarioProgress = 6;
  }
}

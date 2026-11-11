#include "bof3/bof3.h"

void func_801F7EC0(void);
void func_80196070(void);

/* @source 0x801F7CA8
 * @behavior Overlay handler with no caller in this payload: no jal reaches
 *           0x801F7CA8 and no word of the 0x3760-byte payload, aligned or not,
 *           holds that address, so the loader dispatches it by address rather
 *           than through a table word.
 *           It first runs the overlay's record stepper func_801F7EC0
 *           (0x801F7EC0, no arguments, no result), which walks the 0x80 entries
 *           of the 0x28-byte-stride work table at 0x800E4800 and advances,
 *           draws and decays every entry whose byte at offset 0x25 is set.
 *           Only after that call does it re-read the scratch work-object
 *           pointer from the cell at 0x1F800044, because the stepper may
 *           rewrite that cell, and it decrements the work object's word at
 *           +0x0C by one, storing it unconditionally (the store sits in the
 *           branch delay slot, so it happens on both arms). When the stored
 *           value reaches zero it calls the shared work-area reset helper at
 *           0x80196070, which clears bytes 0x00 through 0x04 of the shared work
 *           area and is bound as clearWorkFlags in the target that owns that
 *           body.
 *           The countdown's arming writer inside this overlay is the seeding
 *           handler func_801F7BC0, which calls the same stepper and then stores
 *           0x40 into that word; the word is shared scratchpad space and the
 *           overlay's earlier step-run phase stores a primitive field there
 *           instead.
 *           It takes no arguments, consumes no result, reads nothing else and
 *           its 0x18-byte frame only keeps $ra across the two calls.
 *
 * Name evidence: the stepper call gives the stepping role, the single word this
 *           body decrements and tests gives the countdown, and the shared reset
 *           helper reached only on zero gives the teardown. The function name
 *           and the callee roles come from the original bytes above, not from a
 *           caller, because the payload never names this address.
 *
 * Shape evidence: the 0x48 bytes are byte-identical to the span at 0x801F7DDC
 *           in emi/scenario/scena07/00 except the first jal word, whose callee
 *           is that overlay's own stepper; every other word is equal, so that
 *           asm row seeds the statement order, the cell re-read after the call,
 *           the unconditional word store and the zero-tested guarded tail. The
 *           clone lever is declined on both counts: the differing jal word makes
 *           it a relocation-different sibling rather than a byte-identical one,
 *           and that sibling's own row is still `asm`, so no lifted body exists
 *           to clone. The in-target spelling follows the exact sibling
 *           claimSeedWorkRecordEvery8FramesAndAdvanceWorkByte1AtProgress3ScenarioScena0400_801F7B00,
 *           which calls the same stepper and reads the same cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepActiveWorkRecordsTickWorkCountdownAndResetWorkStateOnZeroScenarioScena0400_801F7CA8(
    void) {
  u8* work;
  s32 count;

  func_801F7EC0();
  work = SPAD_PTR_SLOT(u8, 0x44u);
  count = *(s32*)(work + 0x0C) - 1;
  *(s32*)(work + 0x0C) = count;
  if (count == 0) {
    func_80196070();
  }
}

#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The record's state byte sits at +2 and
 * the dispatcher at 0x801F2C04 indexes this overlay's handler table with it, so
 * the increment below is what selects the next handler. The exact lift of this
 * overlay's entry-1 handler at 0x801F2C94 declares the same cell under this
 * address-derived name.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Shared primary-state byte at 0x80146874 together with the sub-state byte at
 * 0x80146875. This body reads the pair as one halfword, which is the `lhu` the
 * original issues, and compares it with 0x20C: primary state 0x0C with sub-state
 * 2. The main executable's own lifts store the two bytes separately, so the
 * halfword view is this overlay's alone and the cell keeps its address-derived
 * name.
 * @source 0x80146874 @kind unknown
 */
extern u16 D_80146874;

/* Shared cue dispatcher of the main executable, outside this overlay's payload.
 * This target's generated undefined_funcs_auto.txt binds the address, so no
 * target-map row is needed, and the exact world03 lift
 * clearWorkByte9QueueCue202AndAdvanceWorkByte2World03Area11913_801F3D4C takes
 * the same cue argument at the same width.
 * @source 0x8015DF18
 */
extern void func_8015DF18(u16 cue);

/* @source 0x801F2C48
 * @behavior Entry 0 of this overlay's four-word handler table at 0x801F3320,
 *           whose words the payload holds in dispatch order as 0x801F2C48,
 *           0x801F2C94, 0x801F2CDC and 0x801F2D38; the dispatcher at 0x801F2C04
 *           selects one with the work record's unsigned byte +2, reached through
 *           the scratchpad pointer cell 0x1F800044. The handler reads the shared
 *           main-executable halfword at 0x80146874 and compares it with 0x20C,
 *           that is primary state 0x0C in the low byte with sub-state 2 in the
 *           next one. While the pair differs the handler writes nothing and
 *           returns. Once it matches, the handler queues cue 0x202 through the
 *           shared cue dispatcher at 0x8015DF18, then re-reads the scratchpad
 *           pointer and increments the work record's state byte at +2, so the
 *           next dispatch selects entry 1 at 0x801F2C94, the step that draws
 *           this overlay's cell quads. Nothing else is read or written, nothing
 *           is returned, and the 0x18-byte frame only keeps $ra at 0x10 across
 *           the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueCue202AndAdvanceWorkByte2WhenPrimaryState0cSubstate2World02Area10513_801F2C48(
    void) {
  if (D_80146874 == 0x20C) {
    func_8015DF18(0x202);
    D_1F800044[2]++;
  }
}

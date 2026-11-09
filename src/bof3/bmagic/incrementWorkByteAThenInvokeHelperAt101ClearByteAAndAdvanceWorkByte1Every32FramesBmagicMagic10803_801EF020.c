#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The original re-materializes and reloads the cell for each access
 * (lui $v0,0x1F80 / lw $v0,0x44($v0) before the increment and again for each
 * access in the arm), so the cell is reached through this target-map symbol
 * rather than through a constant address, which the compiler would materialize
 * once and reuse.
 * @source 0x1F800044 @kind data
 */
extern u8 *g_battle_work;

/* Main-executable helper at 0x8015D71C. This handler passes the selector that
 * the sibling handlers pass as 0x100/0x102; splat binds 0x8015D71C through this
 * target's generated undefined_funcs_auto.txt, so no target-map row is needed.
 */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* @source 0x801EF020
 * @behavior Third entry (index 2) of this overlay's six-word step-handler run
 *           at 0x801EFC9C, whose words are 0x801EEE0C, 0x801EEF48, 0x801EF020,
 *           0x801EF090, 0x801EF0E0 and 0x801EF130: the per-frame dispatcher
 *           func_801EED7C reads the scratchpad work object's unsigned byte
 *           0x01, scales it by 4, loads that word of the run and jalr's it, so
 *           work byte 0x01 holds 2 while this body runs and no jal in the
 *           payload refers to 0x801EF020. It increments the work object's byte
 *           0xA by one through a byte-width read-modify-write (lbu 0xA / addiu
 *           1 / sb 0xA) and tests the stored value with andi 0x1F, the branch
 *           skipping the arm when the low five bits are nonzero, so the
 *           truncation supplies the tested value and the byte is not re-read.
 *           Only when the count wraps to a multiple of 0x20 - which the arm's
 *           own clear makes every 32nd frame - the handler invokes the
 *           main-executable helper func_8015D71C with selector 0x101, clears
 *           work byte 0xA and increments work byte 0x01 by one at byte width,
 *           advancing the dispatcher to the next entry of the run. Both cell
 *           reads after the call are required, one for the clear and one for
 *           the increment, because the store through the loaded work pointer
 *           may alias the scratchpad cell. The 0x18-byte frame only keeps $ra
 *           across the call; it takes no arguments, returns nothing and reads
 *           or writes no other state. Statement order, operand forms and
 *           declaration style follow the exact siblings
 *           incrementWorkByteAAndAdvanceWorkByte1At60BmagicMagic10803_801EF0E0
 *           (same cell, same byte 0xA read-modify-write, different compared
 *           constant and different arm) and
 *           advanceWorkByte1Every16FramesAndPublishSignedRampToHalfword8014932A
 *           BmagicMagic10203_801EF218 (same masked-countdown arm that clears a
 *           work byte and re-reads the cell to increment work byte 0x01). No
 *           byte-identical copy of this 0x70-byte span exists in any overlay
 *           payload of this repository's binaries; it occurs once, at payload
 *           offset 0x420 of this target's own binary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteAThenInvokeHelperAt101ClearByteAAndAdvanceWorkByte1Every32FramesBmagicMagic10803_801EF020(
    void) {
  u8 countdown;

  countdown = (u8)(g_battle_work[0xA] + 1);
  g_battle_work[0xA] = countdown;
  if ((countdown & 0x1F) == 0) {
    func_8015D71C(0x101);
    g_battle_work[0xA] = 0;
    g_battle_work[1] += 1;
  }
}

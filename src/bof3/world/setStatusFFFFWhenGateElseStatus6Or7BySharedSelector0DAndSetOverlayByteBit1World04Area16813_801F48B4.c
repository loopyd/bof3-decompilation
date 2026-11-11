#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 D_8014933B; /* @source 0x8014933B @kind data */

u32 func_80166140(u32 arg0);

/* @source 0x801F48B4
 * @behavior Overlay state setter reached only from a handler-table word, so it
 *           has no `jal` caller; the payload's trailing pointer run holds its
 *           address at 0x801F4F40, the fifth word of the run that starts at
 *           0x801F4F30 and also names the neighbouring setters 0x801F46F8,
 *           0x801F4764, 0x801F4808, 0x801F4848, 0x801F4920, 0x801F4980 and
 *           0x801F49D4. It loads the shared gate byte at 0x801490C7 (`lb`, so
 *           the cell is s8) and takes one of two paths. While that byte is zero
 *           it queries the shared selector helper func_80166140(0xD): a nonzero
 *           answer stores 6 into the shared status halfword at 0x801490A8
 *           (`addiu v1,0,0x6` in the `beqz` delay slot, then `sh v1`) and sets
 *           bit 1 of the shared overlay byte at 0x8014933B (`lbu`/`ori 0x2`/`sb`),
 *           while a zero answer stores 7 instead (`addiu v0,0,0x7` in the jump
 *           delay slot). While the gate byte is nonzero it stores 0xFFFF into
 *           0x801490A8 (`ori v0,0,0xFFFF`) and touches nothing else. The two
 *           constant arms converge on the one `sh $v0` the original keeps after
 *           them. Takes no arguments and returns nothing; the 0x18-byte frame
 *           only keeps $ra at 0x10 across the call.
 *
 * Name evidence: 0x801490A8 is the shared status halfword the world overlays
 * publish their status codes into, 0x801490C7 is the shared gate byte those
 * handlers branch on, 0x8014933B is the shared overlay byte this payload's own
 * handlers write (setSharedByte1ThenDrawWorld03Area14813_801F3788 stores 1 into
 * it, seedWorkTripletAndOverlayStateBytesWorld04Area19913_801F2D7C clears it),
 * and func_80166140 is the shared selector query the world overlays pass their
 * own selector ids to. The selector id 0xD, the status codes 6, 7 and 0xFFFF and
 * the mask 0x2 are this body's own literals. No role for bit 1 of 0x8014933B or
 * for selector 0xD is evidenced beyond that use, so the name records the writes
 * rather than inventing a game-state role.
 *
 * Shape evidence: this span is the same body as the in-target lifts at
 * 0x801F4848 and 0x801F46F8. A word comparison of the 108-byte spans finds 23
 * of 27 words equal; the four that differ are this body's two discriminating
 * immediates (selector 0xD here against 0xC there, `ori` mask 0x2 here against
 * 0x1 there) and the two absolute `j` words, each shifted by exactly the 0x6C
 * byte delta between this span and 0x801F4848 and by exactly the 0x1BC delta
 * against 0x801F46F8. Its statement order, operand forms and declaration style
 * are transcribed from 0x801F4848, with this body's own selector, mask and
 * status literals written from the original bytes. The span is not
 * byte-identical to either sibling, and a scan of the 396 payloads under
 * out/binaries finds this 108-byte span only in this payload, so it seeded no
 * clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFWhenGateElseStatus6Or7BySharedSelector0DAndSetOverlayByteBit1World04Area16813_801F48B4(
    void) {
  if (D_801490C7 == 0) {
    if (func_80166140(0xD) != 0) {
      D_801490A8 = 6;
      D_8014933B |= 2;
    } else {
      D_801490A8 = 7;
    }
  } else {
    D_801490A8 = 0xFFFF;
  }
}

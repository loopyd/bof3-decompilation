#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 D_8014933B; /* @source 0x8014933B @kind data */

u32 func_80166140(u32 arg0);

/* @source 0x801F4848
 * @behavior Overlay state setter reached only from a handler-table word, so it
 *           has no `jal` caller; the payload's trailing pointer run holds its
 *           address at 0x801F4F30. It loads the shared gate byte at
 *           0x801490C7 (`lb`, so the cell is s8) and takes one of two paths.
 *           While that byte is zero it queries the shared selector helper
 *           func_80166140(0xC): a nonzero answer stores 6 into the shared
 *           status halfword at 0x801490A8 (`addiu v1,0,0x6` in the `beqz`
 *           delay slot, then `sh v1`) and sets bit 0 of the shared overlay byte
 *           at 0x8014933B (`lbu`/`ori 0x1`/`sb`), while a zero answer stores 7
 *           instead (`addiu v0,0,0x7` in the jump delay slot). While the gate
 *           byte is nonzero it stores 0xFFFF into 0x801490A8 (`ori v0,0,0xFFFF`)
 *           and touches nothing else. The two constant arms converge on the one
 *           `sh $v0` the original keeps after them. Takes no arguments and
 *           returns nothing; the 0x18-byte frame only keeps $ra at 0x10 across
 *           the call.
 *
 * Name evidence: 0x801490A8 is the shared status halfword the world overlays
 * publish their status codes into, 0x801490C7 is the shared gate byte those
 * handlers branch on, 0x8014933B is the shared overlay byte this payload's own
 * handlers write (setSharedByte1ThenDrawWorld03Area14813_801F3788 stores 1 into
 * it, seedWorkTripletAndOverlayStateBytesWorld04Area19913_801F2D7C clears it),
 * and func_80166140 is the shared selector query the world overlays pass their
 * own selector ids to. The selector id 0xC and the status codes 6, 7 and 0xFFFF
 * are this body's own literals. No role for bit 0 of 0x8014933B or for
 * selector 0xC is evidenced beyond that use, so the name records the writes
 * rather than inventing a game-state role.
 *
 * Shape evidence: this span is the same body as the in-target lift at
 * 0x801F46F8. A word comparison of the two 108-byte spans finds 25 of 27 words
 * equal and only the two absolute `j` words differing, each by exactly the
 * 0x150 byte delta between the spans (0x807D1D5/0x807D1D3 here,
 * 0x807D229/0x807D227 there), so its statement order, operand forms, literals
 * and declaration style are transcribed from it rather than invented. The
 * span is not byte-identical to that sibling, so it seeded no clone. The
 * neighbouring 0x801F48B4 span carries the same body with selector 0xD and
 * bit 1 of 0x8014933B (`ori v0,v0,0x2`), which is shape evidence that this
 * body's selector 0xC and mask 0x1 come from its own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFWhenGateElseStatus6Or7BySharedSelector0CAndSetOverlayByteBit0World04Area16813_801F4848(
    void) {
  if (D_801490C7 == 0) {
    if (func_80166140(0xC) != 0) {
      D_801490A8 = 6;
      D_8014933B |= 1;
    } else {
      D_801490A8 = 7;
    }
  } else {
    D_801490A8 = 0xFFFF;
  }
}

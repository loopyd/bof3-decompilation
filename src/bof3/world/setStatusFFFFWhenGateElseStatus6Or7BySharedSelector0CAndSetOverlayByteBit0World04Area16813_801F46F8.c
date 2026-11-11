#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 D_8014933B; /* @source 0x8014933B @kind data */

u32 func_80166140(u32 arg0);

/* @source 0x801F46F8
 * @behavior Overlay state setter reached only from a handler-table word, so it
 *           has no `jal` caller. It loads the shared gate byte at 0x801490C7
 *           (`lb`, so the cell is s8) and takes one of two paths. While that
 *           byte is zero it queries the shared selector helper
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
 * Shape evidence: statement order, operand forms and declaration style are
 * taken from the in-target sibling
 * setStatusFFFFAndStateEntryWhenGateClearWorld04Area16813_801F4808 (0x801F4808)
 * for the gated status halfword store and the `lb` gate read, and from
 * setStatus4AndModeIndex6IfGateElseFFFFWorld03Area11613_801F2CCC (0x801F2CCC)
 * for the inverted gate whose 0xFFFF arm is emitted out of line after the
 * selector-status arm. Both are shape evidence rather than clone sources. No
 * byte-identical twin exists: a scan of the 396 payloads under out/binaries
 * finds this 108-byte span only in this payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFWhenGateElseStatus6Or7BySharedSelector0CAndSetOverlayByteBit0World04Area16813_801F46F8(
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

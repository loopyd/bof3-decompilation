#include "bof3/bof3.h"

/* Leading signed byte of the shared main executable's scenario-state record,
 * compared here against 8. It lies outside this EMI payload and is read at the
 * original signed byte width.
 * @source 0x80146870 @kind data
 */
extern s8 D_80146870;

/* Icon sprite emitter of this EMI, lifted exact in this target: it stores the
 * two coordinates as the sprite halfwords at 0x8 and 0xA of the primitive at
 * g_PrimCursor, scales the kind byte by four, takes that entry of the
 * four-byte-stride icon tile table based at 0x801D8B54 for the texture byte
 * pair at +0 and +1 and for the clut word assembled from +2 and +3, sets a
 * 0x28 by 0x30 size, picks the three colour bytes from the mode byte (0 makes
 * all three 0x80, 1 makes them 0x30, any other value gives 0x40 0x40 0x80) and
 * selects the draw-mode texture page with GetGraphType. The widths below are
 * the widths observed at the call site, which widens every argument with andi
 * 0xFFFF/0xFFFF/0xFF/0xFF, not a recovered callee contract.
 * @source 0x801D3518
 */
extern void emitIconKindTile(s32 x, s32 y, s32 kind,
                                                      s32 mode);

/* @source 0x801D3664
 * @behavior Sprite wrapper of the BATE overlay: when the kind byte equals 4 and
 *           the shared scenario-state byte 0x80146870, read with a signed byte
 *           load, is at least 8, it replaces that kind with 0xB. It then widens
 *           both unsigned 16-bit coordinates and both byte arguments and
 *           forwards them to the icon sprite emitter
 *           emitIconKindTile with no other
 *           work; its 0x18-byte frame only keeps $ra at 0x10 across that call.
 *           The 0x54 bytes sit equal under cmp to the emi/etc/shop/00 span at
 *           0x801D910C apart from the single jal word at +0x3C, which moves by
 *           the address delta between the two spans onto each payload's own
 *           local emitter (0x801D3518 here, 0x801D8FC0 there).
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSpriteKindSubstitute(u16 x, u16 y, u8 kind, u8 mode)
{
    if (kind == 4 && D_80146870 >= 8)
    {
        kind = 0xB;
    }
    emitIconKindTile(x, y, kind, mode);
}

#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared render-flags word of the main executable: it lies outside this EMI
 * payload, bit 3 selects which direction the two-bit shading field below is
 * read in, and bits 1 and 2 carry that field.
 * @source 0x80143E6C @kind data
 */
extern s32 D_80143E6C;

/* @source 0x801D3384
 * @behavior Border emitter of the BATE overlay: it builds the three colour
 *           bytes of its two line packets from the low three bits of the mask
 *           argument, each set bit taking the flat colour
 *           0x3F | (((D_80143E6C & 8) ? (D_80143E6C & 6) : (~D_80143E6C & 6)) << 5)
 *           when the colour-enable argument is set and 0xFF when it is zero,
 *           and each clear bit taking black. It then draws the open three-point
 *           flat line from (x + 4, y) to (x, y + 4) to (x, y + h - 1) with
 *           SetLineF3 and the closed four-corner border from (x + 4, y) to
 *           (x + w - 1, y) to (x + w - 1, y + h - 1) to (x, y + h - 1) with
 *           SetLineF4, both packets taken from g_PrimCursor and each queued
 *           through func_8014E5A0(1, size) as 0x18 and 0x1C bytes; its
 *           0x38-byte frame keeps $s0 to $s5 and $ra across those calls. The
 *           0x194 bytes equal under cmp the emi/etc/sisyou/00 span at
 *           0x801D1CC0 and the emi/etc/shop/00 span at 0x801D8E2C apart from
 *           the two j words at +0x60 and +0x88, each of which moves by exactly
 *           the address delta between the two spans, so this is one body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void emitFlatLineRectangleBorderBate00_801D3384(u16 x, u16 y, u16 w, u16 h, u8 colorEnable, u8 mask)
{
    u8 channels[3];
    u32 bit;
    u32 maskBits = mask;
    u16 rightEdge;
    u8 i;
    u32 color;
    s32 flags;
    LINE_F3 *line3;
    LINE_F4 *line4;

    if (colorEnable != 0)
    {
        flags = D_80143E6C;
        if ((flags & 8) != 0)
        {
            color = ((flags & 6) << 5) | 0x3F;
        }
        else
        {
            color = ((~flags & 6) << 5) | 0x3F;
        }
    }
    else
    {
        color = 0xFF;
    }

    bit = 1;
    i = 0;
    do
    {
        if (bit & maskBits)
        {
            channels[i] = color;
        }
        else
        {
            channels[i] = 0;
        }
        i++;
        bit <<= 1;
    } while (i < 3);

    line3 = (LINE_F3 *)g_PrimCursor;
    SetLineF3(line3);
    line3->x0 = x + 4;
    line3->y0 = y;
    line3->x1 = x;
    line3->y1 = y + 4;
    line3->x2 = x;
    line3->y2 = y + h - 1;
    line3->r0 = channels[2];
    line3->g0 = channels[1];
    line3->b0 = channels[0];
    func_8014E5A0(1, 0x18);

    line4 = (LINE_F4 *)g_PrimCursor;
    SetLineF4(line4);
    rightEdge = x + w - 1;
    line4->x0 = x + 4;
    line4->y0 = y;
    line4->x3 = x;
    line4->y3 = y + h - 1;
    line4->y1 = y;
    line4->y2 = y + h - 1;
    line4->x2 = rightEdge;
    line4->x1 = rightEdge;
    line4->r0 = channels[2];
    line4->g0 = channels[1];
    line4->b0 = channels[0];
    func_8014E5A0(1, 0x1C);
}

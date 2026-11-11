#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared render-flags word of the main executable, outside this EMI payload:
 * bit 3 selects the direction the two-bit shading field in bits 1 and 2 is read
 * in, and that field's doubled value forms the flat colour this body writes.
 * @source 0x80143E6C @kind data */
extern s32 D_80143E6C;

/* @source 0x801D432C
 * @behavior Flat-line border emitter of the SHISU overlay: it builds the three
 *           colour bytes of its two line packets from the low three bits of the
 *           mask argument, each set bit taking the flat colour
 *           0x3F | (((D_80143E6C & 8) ? (D_80143E6C & 6) : (~D_80143E6C & 6)) << 5)
 *           when the colour-enable argument is set and 0xFF when that argument
 *           is zero, and each clear bit taking black. It then draws the open
 *           three-point flat line from (x + 4, y) to (x, y + 4) to (x, y + h - 1)
 *           with SetLineF3 and the closed four-corner border from (x + 4, y) to
 *           (x + w - 1, y) to (x + w - 1, y + h - 1) to (x, y + h - 1) with
 *           SetLineF4, both packets taken from g_PrimCursor and each queued
 *           through func_8014E5A0(1, size) as 0x18 and 0x1C bytes. SetLineF3,
 *           SetLineF4 and func_8014E5A0 are calls of the main executable, so
 *           this span carries no payload-local target. No jal and no word of
 *           this payload names it, so it is an overlay entry called from
 *           outside the payload, and its 0x38-byte frame keeps $s0 through $s5
 *           and $ra across those calls. One payload cmp proves its 0x194 bytes
 *           equal the emitFlatLineRectangleBorderBate00_801D3384 span at
 *           0x801D3384 in emi/etc/bate/00 and the span at 0x801D8E2C in
 *           emi/etc/shop/00 apart from the two internal j words at +0x60 and
 *           +0x88, each of which moves by exactly the address delta between the
 *           spans, so this is one body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void emitFlatLineRectangleBorderShisu00_801D432C(u16 x, u16 y, u16 w, u16 h, u8 colorEnable,
                                                 u8 mask)
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

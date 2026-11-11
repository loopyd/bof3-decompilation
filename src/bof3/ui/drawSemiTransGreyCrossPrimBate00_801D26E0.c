#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Engine primitive initialiser of the main executable: it writes the tag and the
 * code byte of the packet that g_PrimCursor points at. This payload only calls
 * it, and the address lies outside every EMI payload.
 * @source 0x8017AA80 @kind function */
extern void func_8017AA80(void *primitive);

/* @source 0x801D26E0
 * @behavior Draws a mid-grey semi-transparent cross marker at the screen
 *           position it is given, as two 0x10-byte LINE_F2 primitives taken
 *           from g_PrimCursor. Each half initialises its packet through
 *           func_8017AA80, marks it semi-transparent, fills the four corner
 *           halfwords at 0x08 to 0x0E, writes 0x80 into the three colour bytes
 *           at 0x04 to 0x06 and appends the packet as a 0x10-byte primitive at
 *           ordering-table slot 1 through func_8014E5A0. The first packet is
 *           the vertical stroke from (x + 5, y) to (x + 5, y + 10) and the
 *           second, after a fresh read of g_PrimCursor, the horizontal stroke
 *           from (x, y + 5) to (x + 10, y + 5), so the two arms cross at the
 *           centre (x + 5, y + 5) with five pixels on each side. Both helper
 *           addresses lie outside this EMI payload; the payload's only call to
 *           this span is in func_801D2304, which passes its own x argument plus
 *           0x94 and its row y. Its 0x28-byte frame keeps $s0 to $s3 and $ra
 *           across the four calls.
 * @status partial
 * @match 98.25
 * @residual one scheduling slot: the pre-reload scheduler hoists the shared
 *           x + 5 addiu two instructions above its position in the original, so
 *           56 of 57 instructions match at the original 228-byte size
 */
void drawSemiTransGreyCrossPrimBate00_801D26E0(s16 x, s16 y)
{
    LINE_F2 *primitive;
    s16 stroke_x;

    primitive = (LINE_F2 *)g_PrimCursor;
    func_8017AA80(primitive);
    SetSemiTrans(primitive, 1);
    stroke_x = x + 5;
    primitive->y0 = y;
    primitive->x0 = stroke_x;
    primitive->x1 = stroke_x;
    primitive->y1 = y + 10;
    primitive->r0 = primitive->g0 = primitive->b0 = 0x80;
    func_8014E5A0(1, 0x10);
    primitive = (LINE_F2 *)g_PrimCursor;
    func_8017AA80(primitive);
    SetSemiTrans(primitive, 1);
    primitive->x0 = x;
    primitive->y0 = y + 5;
    primitive->x1 = x + 10;
    primitive->y1 = y + 5;
    primitive->r0 = primitive->g0 = primitive->b0 = 0x80;
    func_8014E5A0(1, 0x10);
}

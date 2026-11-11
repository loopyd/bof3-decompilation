#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared graphic-type query. The draw-mode texture page this body installs is
 * selected from it, and no header in this tree declares it. */
extern s32 GetGraphType(void);

/* @source 0x801D0F50
 * @behavior Emits one 0xC-by-0xC textured icon sprite at (arg0, arg1) into the
 *           shared primitive cursor: a draw-mode primitive carrying the
 *           graph-type texture page (0x2E for graph type 1, 0x2E for graph type
 *           2 and 0x1E otherwise) installed with dfe 0 and dtd 0, a 0xC-byte
 *           packet, then a SPRT whose texture column is arg2 * 12 - 0x38, whose
 *           texture row is 0xF0, whose CLUT is 0x7849, whose size is 0xC by
 *           0xC, and whose colour is the neutral gray 0x80 on all three
 *           channels, closed by a 0x14-byte packet. The icon index is arg2
 *           masked to its low byte. Both in-image callers fill one row of this
 *           overlay's fairy list and pass the icon at (x + 8, y + 0x14): the row
 *           at 0x801D1048 passes 1 and the row at 0x801D129C passes 0. It reads
 *           no other memory, writes only the cursor, and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawRowIconSpriteCommu0507(s16 arg0, s16 arg1, u8 arg2) {
    SPRT *sprite;
    s32 page;
    u32 kind;

    if (GetGraphType() == 1) {
        page = 0x2E;
    } else if (GetGraphType() == 2) {
        page = 0x2E;
    } else {
        page = 0x1E;
    }

    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 0, page, 0);
    GpuAppendPrim(1, 0xC);

    sprite = (SPRT *)g_PrimCursor;
    SetSprt(sprite);

    kind = arg2 & 0xFF;
    sprite->x0 = arg0;
    sprite->y0 = arg1;
    sprite->u0 = kind * 12 - 0x38;
    sprite->v0 = 0xF0;
    sprite->w = 0xC;
    sprite->h = 0xC;
    sprite->clut = 0x7849;
    sprite->r0 = 0x80;
    sprite->g0 = 0x80;
    sprite->b0 = 0x80;
    GpuAppendPrim(1, 0x14);
}

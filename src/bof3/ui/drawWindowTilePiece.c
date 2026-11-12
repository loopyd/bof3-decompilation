#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Graphics-type query of the running executable, owned by the SDK map at
 * 0x8017B2B4. This payload only calls it. */
extern s32 GetGraphType(void);

/* Six-byte entries of the commu01/07 tile atlas, one per piece: the atlas cell
 * u and v, the sprite width and height, the palette column scaled by 16 and the
 * palette row offset. Pieces 6 through 13 are the 8x8 window tiles that
 * drawWindowFrameCommu0107_801D3594, drawSmallWindowFrameCommu0107_801D273C and
 * drawEntryListFrame repeat around their frames. */
extern u8 D_801D4CE0[]; /* @source 0x801D4CE0 @kind data */

/* Representation notes, each measured against this body's 82 instructions at
 * 328 bytes: writing the three colour stores ahead of the CLUT store scored 53
 * of 82 at 324 bytes, inline piece * 6 index expressions scored 70 of 82 with
 * the CLUT first and 41 of 82 at 324 bytes with the colours first, and a
 * six-byte entry struct pointer scored 64 of 82 at 292 bytes with the CLUT
 * first and 50 of 82 at 288 bytes with the colours first. The byte-offset local
 * written below with the CLUT store ahead of the three colour stores scored 82
 * of 82 at the original size. */

/* @source 0x801D4030
 * @behavior Emits one atlas piece of the commu01/07 entry screen as a free-size
 *           sprite at (x, y) into the shared primitive cursor. GetGraphType
 *           selects the draw-mode texture page, keeping 0x2E when a query
 *           answers 1, 0x2E when a second query answers 2 and 0x1E otherwise;
 *           the page is installed with dfe 0, dtd 0 and no texture window, and
 *           that 0xC-byte draw-mode packet is appended at ordering-table slot
 *           1. SetSprt then initialises the 0x14-byte sprite at the advanced
 *           cursor, which takes the position from the two coordinates and the
 *           six bytes of entry piece of D_801D4CE0 as its cell u and v, its
 *           width and height and its CLUT, the CLUT being the palette row
 *           offset plus 0x1E0 shifted up by six and ORed with the palette
 *           column shifted down by four. All three colour bytes are the neutral
 *           gray 0x80 and the sprite is appended at slot 1 as 0x14 bytes. The
 *           0x30-byte frame keeps $s0 through $s3 and $ra across the five calls.
 *           Its three in-image callers drawWindowFrameCommu0107_801D3594,
 *           drawSmallWindowFrameCommu0107_801D273C and drawEntryListFrame pass
 *           pieces 6 through 13, the corner and edge tiles of their frames; no
 *           jal and no pointer word outside the payload names this span, so its
 *           argument roles are structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWindowTilePiece(s16 x, s16 y, u8 piece) {
    s32   offset;
    s32   page;
    SPRT* sprite;

    if (GetGraphType() == 1) {
        page = 0x2E;
    } else if (GetGraphType() == 2) {
        page = 0x2E;
    } else {
        page = 0x1E;
    }

    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, page, 0);
    GpuAppendPrim(1, 0xC);

    sprite = (SPRT*)g_PrimCursor;
    SetSprt(sprite);

    offset = piece * 6;
    sprite->x0 = x;
    sprite->y0 = y;
    sprite->u0 = D_801D4CE0[offset + 0];
    sprite->v0 = D_801D4CE0[offset + 1];
    sprite->w = D_801D4CE0[offset + 2];
    sprite->h = D_801D4CE0[offset + 3];
    sprite->clut = ((D_801D4CE0[offset + 5] + 0x1E0) << 6) | (D_801D4CE0[offset + 4] >> 4);
    sprite->r0 = 0x80;
    sprite->g0 = 0x80;
    sprite->b0 = 0x80;
    GpuAppendPrim(1, 0x14);
}

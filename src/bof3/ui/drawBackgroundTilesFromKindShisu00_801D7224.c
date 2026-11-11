#include "bof3/bof3.h"
#include "gpu/prim.h"

/* One cell of the SHISU tile atlas every sprite of this body takes its texture
 * rectangle from: the two origin bytes of the cell inside the atlas and the
 * cell size, which is 0x20 wide and 0x10 or 0x20 tall. The run holds 0x2B
 * records of four bytes each and only this body reads it, so it keeps the
 * generated name of its address.
 */
typedef struct Shisu00TileCell {
    u8 u;
    u8 v;
    u8 w;
    u8 h;
} Shisu00TileCell;

/* @source 0x801D9D4C @kind table */
extern Shisu00TileCell D_801D9D4C[];

/* The tile-index blob: four runs of one byte per atlas cell, each run starting
 * at the offset D_801D9E44 holds for its kind. A column of the background walks
 * one run in order and steps down by the height of the cell it just emitted.
 * @source 0x801D9DF8 @kind table */
extern u8 D_801D9DF8[];

/* Start offsets of the four index runs of D_801D9DF8, 0x00, 0x16, 0x2A and
 * 0x3C. The body indexes it with the low byte of its argument.
 * @source 0x801D9E44 @kind table */
extern u8 D_801D9E44[];

/* @source 0x801D7224
 * @behavior Paints the 0x140-by-0xF0 background of the SHISU overlay with atlas
 *           cells selected by the low byte of the argument, which picks both
 *           the CLUT and the index run: GetTPage(0, 0, 0x340, 0x100) becomes
 *           the texture page of a draw-mode packet installed at g_PrimCursor
 *           with dfe 0, dtd 0 and no texture window and appended at slot 1 as
 *           0xC bytes, and four CLUTs taken from row 0x1E2 at x 0x30, 0x50,
 *           0x40 and 0x60 fill a four-halfword local run of which the argument
 *           selects one entry. Ten 0x20-wide columns then follow from x 0 in
 *           five passes of two: each pass restarts the index run of its kind at
 *           D_801D9DF8 plus the byte of D_801D9E44 the argument selects, and
 *           each column stacks sprites from y 0 down the screen while the
 *           running y stays below 0xF0, stepping y by the height of the cell it
 *           emitted and moving one byte along the run. Every sprite is
 *           initialised by SetSprt on g_PrimCursor, marked semi transparent
 *           with 0, given the column x and the running y as the two signed
 *           coordinates, the selected CLUT halfword, the first two bytes of the
 *           atlas cell as its origin, its size as w and h, and 0x80 in the
 *           three colour bytes, then appended at slot 1 as 0x14 bytes. No jal
 *           and no table word of this payload names this body, so it is an
 *           overlay entry called from outside the payload and its argument role
 *           stays structural. The three data references are the only words that
 *           differ from this same body at 0x801D627C in emi/etc/bate/00 and at
 *           0x801DBD24 in emi/etc/shop/00, each shifted by exactly the distance
 *           between the payloads, so those rows are shape evidence and not a
 *           clone source. Its 0x50-byte frame keeps the eight saved registers
 *           and $ra and the epilogue returns no value.
 * @status partial
 * @match 92.80
 * @residual The body reaches the original's 0x1F4 bytes and matches 116 of its
 *           125 instructions. The unclosed remainder is one instruction moved
 *           and the register pair it drags with it: the comment above traces
 *           the 0x80 tint literal's only load into a register that lives across
 *           the whole function, where the original reloads it at the head of the
 *           two-pass loop, so the column x takes s5 here against s6 there and
 *           every back-edge target below that head is one instruction lower.
 *           Measured spellings of that literal - assigned before the loops, at
 *           the head, inside the innermost body, block-scoped, chained against
 *           three separate stores, and wrapped in a single-iteration scope -
 *           all reproduce the same relocation at the same size.
 */
void drawBackgroundTilesFromKindShisu00_801D7224(u32 kind) {
    SPRT* prim;
    Shisu00TileCell* cell;
    u8* run;
    u16 cluts[4];
    u8 shade;
    u16* clut;
    u32 kindIndex;
    s16 x;
    s16 y;
    u8 col;
    u8 row;

    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, 0x340, 0x100), 0);
    GpuAppendPrim(1, 0xC);

    cluts[0] = GetClut(0x30, 0x1E2);
    cluts[1] = GetClut(0x50, 0x1E2);
    cluts[2] = GetClut(0x40, 0x1E2);
    cluts[3] = GetClut(0x60, 0x1E2);

    kindIndex = kind & 0xFF;
    shade = 0x80;
    clut = (u16*)(kindIndex * 2 + (u32)cluts);
    x = 0;

    for (col = 0; col < 5; col++) {
        run = &D_801D9DF8[D_801D9E44[kindIndex]];
        for (row = 0; row < 2; row++) {
            y = 0;
            while (y < 0xF0) {
                prim = (SPRT*)g_PrimCursor;
                SetSprt(prim);
                SetSemiTrans(prim, 0);

                prim->x0 = x;
                prim->y0 = y;
                prim->clut = *clut;

                cell = &D_801D9D4C[*run];
                prim->u0 = cell->u;
                prim->v0 = cell->v;
                prim->w = cell->w;
                prim->h = cell->h;
                prim->r0 = shade;
                prim->g0 = shade;
                prim->b0 = shade;

                GpuAppendPrim(1, 0x14);
                y += cell->h;
                run++;
            }
            x += 0x20;
        }
    }
}

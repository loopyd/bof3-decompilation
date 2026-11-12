#include "bof3/bof3.h"

/* Shaded-box filler of the resident executable: paints the rectangle at (x, y)
 * with the given size, shade byte and clut byte. */
extern void func_801AE3F0(u16 x, u16 y, u32 width, u32 height, u32 shade,
                          u32 clut); /* @source 0x801AE3F0 */

/* Text emitter of the resident executable: draws the cells of the 5-byte-stride
 * image row it is handed at (x, y). */
extern void func_8014F800(s16 x, s16 y, s32 arg2, s32 arg3,
                          u32 image); /* @source 0x8014F800 */

/* Primitive append helper: appends the primitive the shared cursor holds to the
 * given ordering-table slot with the given byte size. */
extern void func_8014E5A0(u32 ot_index,
                          u32 primitive_size); /* @source 0x8014E5A0 */

/* The shared primitive cursor the engine publishes for the overlay emitters. */
extern u8 *g_PrimCursor; /* @source 0x8014598C @kind bss */

/* Shared clut byte of the panel the resident executable paints. */
extern u8 D_80144952; /* @source 0x80144952 @kind data */

/* Request row byte of the current commu request: zero while no row is pending,
 * otherwise the record kind less 4. This body reads it as a signed byte. */
extern s8 D_801D4E28; /* @source 0x801D4E28 @kind data */

/* Kind byte of each of the eight stride-8 local record slots. */
extern u8 D_801457A8[][8]; /* @source 0x801457A8 @kind data */

/* Channel table indexed by record kind: the channel 0 through 3 this body
 * blinks while a row is pending. */
extern u8 D_801D4C20[]; /* @source 0x801D4C20 @kind data */

/* The four colour-channel bytes of each of the sixty stride-9 records that
 * start at 0x801F2705: one byte per red, green, blue and cyan channel. */
extern u8 D_801F2705[]; /* @source 0x801F2705 @kind data */
extern u8 D_801F2706[]; /* @source 0x801F2706 @kind data */
extern u8 D_801F2707[]; /* @source 0x801F2707 @kind data */
extern u8 D_801F2708[]; /* @source 0x801F2708 @kind data */

void drawSmallWindowFrameCommu0107_801D273C(s16 x, s16 y);
void drawPaletteChannelGradientBarCommu0107_801D2D98(s16 x, s16 y, s32 width,
                                                     u8 channel, u8 blink);

/* Representation notes: the direct spellings measured worse than the derived
 * base. Naming the clut byte and the name rows directly measured 280 of 318
 * instructions at four bytes over, the clut base alone 289 of 318 at the same
 * surplus, and a base with the row offset folded into one subscript
 * (`block[0xE96 + slot * 5]`) 239 of 317 with the slot spilled to a 0x50 frame.
 * Holding the derived row base in its own pointer measures 311 of 317 at the
 * original size and its explicit scaled offset 312 of 317, both order only. The
 * kind chain is the remaining order: with the subtract arm second the compiler
 * inverts the middle test and normalises the byte on the else edge, while
 * writing that arm first measured 317 of 317. A signed kind local masked at the
 * channel index measured 268 of 317 at 1264 bytes. */

/* @source 0x801D28A4
 * @behavior Draws the channel-weight box of one commu record slot at the
 * top-left corner (x, y). It fills two shaded boxes with the shared clut byte
 * 0x80144952: the inner one at (x + 4, y + 4) spanning 0x48 by 0x28 under shade
 * 0x81 and the outer one at (x, y) spanning 0x48 by 0x26 under shade 0x80, and
 * then draws the small window frame at (x, y). It stamps a 0x3C by 0x10 tile at
 * the shared primitive cursor from the 5-byte-stride name row 0x801457E8 the
 * slot selects, positioned at (x + 6, y + 0x12) and appended under
 * ordering-table slot 1 with size 0x10. Four gradient bars follow at (x + 6, y +
 * 0x12), (x + 6, y + 0x16), (x + 6, y + 0x1A) and (x + 6, y + 0x1E), one per
 * channel 0 through 3, each as wide as twelve times the slot's stride-9 channel
 * byte at 0x801F2705, 0x801F2706, 0x801F2707 and 0x801F2708. While the selected
 * byte is zero all four bars take the fixed brightness. Otherwise the request
 * row byte 0x801D4E28 selects the record kind, zero for row zero, the kind byte
 * of the stride-8 slot run 0x801457A8 at row minus one for rows 1 through 8 and
 * row minus 8 for a row of 9 or more, and that kind's channel in the table at
 * 0x801D4C20
 * picks the single bar that blinks while the other three take the fixed
 * brightness. No other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawRecordSlotChannelBox(s16 x, s16 y, u8 slot, s32 selected) {
    TILE *prim;
    u8 *block;
    u8 *names;
    u32 offset;
    s8 row;
    u8 sel;
    u8 kind;

    /* The shared panel block: the clut byte leads it and the 5-byte name rows of
     * 0x801457E8 sit 0xE96 bytes above it. */
    block = &D_80144952;
    func_801AE3F0((u16)(x + 4), (u16)(y + 4), 0x48, 0x28, 0x81, block[0]);
    func_801AE3F0((u16)x, (u16)y, 0x48, 0x26, 0x80, block[0]);
    drawSmallWindowFrameCommu0107_801D273C(x, y);
    offset = slot * 5;
    names = block + 0xE96;
    func_8014F800((s16)(x + 6), (s16)(y + 4), 0, 5, (u32)&names[offset]);
    prim = (TILE *)g_PrimCursor;
    SetTile(prim);
    prim->x0 = x + 6;
    prim->y0 = y + 0x12;
    prim->w = 0x3C;
    prim->h = 0x10;
    prim->r0 = 0;
    prim->g0 = 0;
    prim->b0 = 0;
    func_8014E5A0(1, 0x10);
    sel = selected;
    if (sel == 0) {
        drawPaletteChannelGradientBarCommu0107_801D2D98(
            (s16)(x + 6), (s16)(y + 0x12), D_801F2705[slot * 9] * 12, 0, 0);
        drawPaletteChannelGradientBarCommu0107_801D2D98(
            (s16)(x + 6), (s16)(y + 0x16), D_801F2706[slot * 9] * 12, 1, 0);
        drawPaletteChannelGradientBarCommu0107_801D2D98(
            (s16)(x + 6), (s16)(y + 0x1A), D_801F2707[slot * 9] * 12, 2, 0);
        drawPaletteChannelGradientBarCommu0107_801D2D98(
            (s16)(x + 6), (s16)(y + 0x1E), D_801F2708[slot * 9] * 12, 3, 0);
    } else {
        row = D_801D4E28;
        if (row == 0) {
            kind = 0;
        } else if (row > 8) {
            kind = row - 8;
        } else {
            kind = D_801457A8[row - 1][0];
        }
        if (D_801D4C20[kind] == 0) {
            drawPaletteChannelGradientBarCommu0107_801D2D98(
                (s16)(x + 6), (s16)(y + 0x12), D_801F2705[slot * 9] * 12, 0, 1);
        } else {
            drawPaletteChannelGradientBarCommu0107_801D2D98(
                (s16)(x + 6), (s16)(y + 0x12), D_801F2705[slot * 9] * 12, 0, 0);
        }
        if (D_801D4C20[kind] == 1) {
            drawPaletteChannelGradientBarCommu0107_801D2D98(
                (s16)(x + 6), (s16)(y + 0x16), D_801F2706[slot * 9] * 12, 1, 1);
        } else {
            drawPaletteChannelGradientBarCommu0107_801D2D98(
                (s16)(x + 6), (s16)(y + 0x16), D_801F2706[slot * 9] * 12, 1, 0);
        }
        if (D_801D4C20[kind] == 2) {
            drawPaletteChannelGradientBarCommu0107_801D2D98(
                (s16)(x + 6), (s16)(y + 0x1A), D_801F2707[slot * 9] * 12, 2, 1);
        } else {
            drawPaletteChannelGradientBarCommu0107_801D2D98(
                (s16)(x + 6), (s16)(y + 0x1A), D_801F2707[slot * 9] * 12, 2, 0);
        }
        if (D_801D4C20[kind] == 3) {
            drawPaletteChannelGradientBarCommu0107_801D2D98(
                (s16)(x + 6), (s16)(y + 0x1E), D_801F2708[slot * 9] * 12, 3, 1);
        } else {
            drawPaletteChannelGradientBarCommu0107_801D2D98(
                (s16)(x + 6), (s16)(y + 0x1E), D_801F2708[slot * 9] * 12, 3, 0);
        }
    }
}

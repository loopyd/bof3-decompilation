#include "bof3/bof3.h"

/* The engine helpers this overlay emitter calls: 0x8017A9CC is the POLY_G4
 * initialiser that stamps the primitive the shared cursor points at, and
 * 0x8014E5A0 appends a primitive of the given size class to the ordering
 * table. */
extern void SetPolyG4(POLY_G4* prim); /* @source 0x8017A9CC */
/* @source 0x8014E5A0 */
extern void func_8014E5A0(u32 ot_index, u32 primitive_size);

/* The shared primitive cursor the engine publishes for the overlay emitters. */
extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */

/* Shared frame counter owned by the main exe; its low bits select the blink
 * phase of a blinking bar. */
extern s32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* The four-entry channel palette that leads this overlay's payload: one byte
 * per red, green and blue channel of each entry, red, green, blue and cyan in
 * table order. */
typedef struct {
    u8 rgb[4][3];
} Commu0107ChannelPalette;

extern Commu0107ChannelPalette D_801D0C04; /* @source 0x801D0C04 @kind data */

/* @source 0x801D2D98
 * @behavior Emits one coloured bar of the commu01/07 entry-detail panel at the
 * shared primitive cursor, width pixels wide and four pixels tall. It copies
 * the overlay's four-entry channel palette at 0x801D0C04 into a local and
 * takes the entry the channel byte selects: entry 0 is red, 1 green, 2 blue
 * and 3 cyan, one byte per channel. Brightness is a fixed 0x80 when the blink
 * byte is clear; otherwise it takes bits 1 and 2 of the shared frame counter
 * 0x80143E6C, scaled by 32 and offset by 0x3F, inverted while bit 3 of that
 * counter is clear, so the bar blinks. It then stamps the cursor through the
 * POLY_G4 initialiser at 0x8017A9CC and writes two 0x24-byte gouraud quads:
 * the first spans (x, y) to (x + width, y + 2) with its top edge at a quarter
 * of the selected colour and its bottom edge at the full colour, and the
 * second spans (x, y + 2) to (x + width, y + 4) with its top edge at the full
 * colour and its bottom edge at a quarter of it, so the pair is one bar whose
 * bright core sits at y + 2. Each quad repeats the first vertex pair's x on
 * the second pair, and func_8014E5A0 appends it under ordering-table slot 1.
 * It returns nothing and writes no global state beyond the primitive. Its x, y and
 * channel arguments are the 16-bit and byte quantities its callers sign-extend or
 * mask before the call.
 * @status partial
 * @match 89.29
 * @residual 150 of 168 instructions. The instruction set, register allocation, frame
 * size and byte count match; the two remaining regions are order only. The original
 * interleaves each moved parameter's save with its move in parameter order, which also
 * frees $a1 and $a2 as scratch for the palette copy, while the compiler here emits the
 * saves in its own order and schedules the $s6 and $s5 moves after the copy block. In
 * the first quad the y + 2 store follows the y + 2 computation in the original and
 * trails two stores here. No source spelling tried moved either region: six sweeps
 * covering the copy form (struct assignment, initializer, memcpy, byte array plus cast,
 * array global, source pointer), the vertex statement order (vertex order, all x before
 * all y, emitted order, second pair grouped) and direct palette indexing measured the
 * same 150 or worse.
 */
void drawPaletteChannelGradientBarCommu0107_801D2D98(s16 x, s16 y, s32 width,
                                                     u8 channel, u8 blink) {
    Commu0107ChannelPalette palette;
    POLY_G4* prim;
    u8* rgb;
    u32 brightness;
    u32 counter;

    palette = D_801D0C04;
    if (blink != 0) {
        counter = D_80143E6C;
        if (counter & 8) {
            brightness = ((counter & 6) << 5) | 0x3F;
        } else {
            brightness = ((~counter & 6) << 5) | 0x3F;
        }
    } else {
        brightness = 0x80;
    }

    prim = (POLY_G4*)g_PrimCursor;
    SetPolyG4(prim);
    prim->x0 = x;
    prim->y0 = y;
    prim->x1 = x + width;
    prim->y1 = y;
    prim->x2 = prim->x0;
    prim->y2 = y + 2;
    prim->x3 = prim->x1;
    prim->y3 = y + 2;
    rgb = palette.rgb[channel];
    prim->r2 = rgb[0] * brightness;
    prim->g2 = rgb[1] * brightness;
    prim->b2 = rgb[2] * brightness;
    prim->r0 = prim->r2 >> 2;
    prim->g0 = prim->g2 >> 2;
    prim->b0 = prim->b2 >> 2;
    prim->r3 = prim->r2;
    prim->g3 = prim->g2;
    prim->b3 = prim->b2;
    prim->r1 = prim->r0;
    prim->g1 = prim->g0;
    prim->b1 = prim->b0;
    func_8014E5A0(1, 0x24);

    prim = (POLY_G4*)g_PrimCursor;
    SetPolyG4(prim);
    prim->x0 = x;
    prim->y0 = y + 2;
    prim->x1 = x + width;
    prim->y1 = y + 2;
    prim->x2 = prim->x0;
    prim->y2 = y + 4;
    prim->x3 = prim->x1;
    prim->y3 = y + 4;
    prim->r0 = rgb[0] * brightness;
    prim->g0 = rgb[1] * brightness;
    prim->b0 = rgb[2] * brightness;
    prim->r2 = prim->r0 >> 2;
    prim->g2 = prim->g0 >> 2;
    prim->b2 = prim->b0 >> 2;
    prim->r1 = prim->r0;
    prim->g1 = prim->g0;
    prim->b1 = prim->b0;
    prim->r3 = prim->r2;
    prim->g3 = prim->g2;
    prim->b3 = prim->b2;
    func_8014E5A0(1, 0x24);
}

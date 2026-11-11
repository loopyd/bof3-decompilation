#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared graphic-type query. The draw-mode texture page this body installs is
 * selected from it, and no header in this tree declares it. */
extern s32 GetGraphType(void);

/* Cell drawer of this overlay at 0x801D1670, still an assembly row here. Its
 * own body shows the shape it takes: SetSprt on the primitive cursor, then a
 * halfword x and y, the byte pair it stores at sprite offsets 0xC and 0xD, a
 * clut of 0x7883, and a width of 0x10 for a fifth stack argument of 0 plus
 * 0x10 per unit, so the fifth argument selects a 0x10-wide or 0x20-wide cell
 * and its height is the fixed 0x10. The two origin bytes travel as full words:
 * the original hands them over without a byte truncation, so only the low byte
 * of each reaches the sprite. */
/* @source 0x801D1670 */
extern void func_801D1670(s16 x, s16 y, s32 u, s32 v, u8 wide);

/* Texture-origin pair of one cell of the panel grid: the two bytes the cell
 * drawer stores into the sprite, one pair per cell. The three runs the three
 * bands of the grid walk are adjacent in this overlay's data, each starting on
 * a two-byte boundary, and each is a separate base in the original (every read
 * forms its own lui/addiu of that base added to a two-byte-stride index). */
typedef struct Commu0507CellOrigin {
    u8 u;
    u8 v;
} Commu0507CellOrigin;

/* @source 0x801D199C @kind table */
extern Commu0507CellOrigin D_801D199C[];
/* @source 0x801D19BC @kind table */
extern Commu0507CellOrigin D_801D19BC[];
/* @source 0x801D19DC @kind table */
extern Commu0507CellOrigin D_801D19DC[];

/* @source 0x801D1724
 * @behavior Paints this overlay's 0x100-by-0xC0 panel background as three bands
 *           of sprite cells at (arg0, arg1), the origin its only in-image
 *           caller func_801D0D4C at 0x801D0D4C passes as (0x20, 0x18). It
 *           first selects the draw-mode texture page 0x2D when the shared
 *           graphic type is 1 or 2 and 0x1D otherwise, installs it at
 *           g_PrimCursor with dfe 0 and dtd 0, and links a 0xC-byte draw-mode
 *           packet. The top band is sixteen 0x10-wide cells on a 0x10 pitch at
 *           y = arg1 reading the sixteen origins at 0x801D199C; the bottom band
 *           repeats that row at y = arg1 + 0xB0 reading the sixteen origins at
 *           0x801D19BC; the middle band walks ten origins at 0x801D19DC as ten
 *           rows on a 0x10 pitch from arg1 + 0x10 and draws, for each of four
 *           columns on a 0x40 pitch, one 0x20-wide cell at x = arg0 + column
 *           and one at x + 0x20, taking the first cell origin straight from the
 *           pair and packing the second into u = (byte & 1) << 5 and
 *           v = (byte << 3) & 0xF0. Every cell is handed to the local cell
 *           drawer at 0x801D1670 with the wide flag 0 for the two outer bands
 *           and 1 for the middle one. It reads no other memory, writes none,
 *           and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawPanelBackgroundTilesCommu0507(u16 arg0, u16 arg1) {
    s32 i;
    s32 j;
    s32 page;
    s16 y;
    s32 dy;

    if (GetGraphType() == 1 || GetGraphType() == 2) {
        page = 0x2D;
    } else {
        page = 0x1D;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 0, page, 0);
    GpuAppendPrim(1, 0xC);

    for (i = 0; i < 0x10; i++) {
        func_801D1670(arg0 + i * 0x10, arg1, D_801D199C[i].u, D_801D199C[i].v,
                      0);
    }

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 0xA; j++) {
            /* Row pitch of the middle band, one cell height below its top. */
            dy = j * 0x10 + 0x10;
            y = arg1 + dy;
            func_801D1670(arg0 + i * 0x40, y,
                          (D_801D19DC[j].u & 1) << 5,
                          (D_801D19DC[j].u << 3) & 0xF0, 1);
            func_801D1670(arg0 + i * 0x40 + 0x20, y,
                          (D_801D19DC[j].v & 1) << 5,
                          (D_801D19DC[j].v << 3) & 0xF0, 1);
        }
    }

    for (i = 0; i < 0x10; i++) {
        func_801D1670(arg0 + i * 0x10, arg1 + 0xB0, D_801D19BC[i].u,
                      D_801D19BC[i].v, 0);
    }
}

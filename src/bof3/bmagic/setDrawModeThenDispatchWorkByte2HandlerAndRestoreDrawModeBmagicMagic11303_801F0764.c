#include "bof3/bof3.h"
#include "gpu/prim.h"

extern s32 GetGraphType(void);
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u32 D_801459F0; /* @source 0x801459F0 @kind unknown */
extern void (*D_801F0BD8[])(void); /* @source 0x801F0BD8 @kind data */

/* @source 0x801F0764
 * @behavior Selects draw page 0x2A5 for graphics types 1 or 2, otherwise
 * 0xB5, using successive GetGraphType calls. Sets a draw-mode packet at
 * the primitive cursor with dfe 0, dtd 1 and no texture window, then appends
 * its 0xC bytes under ordering-table slot 3. Publishes 0x800C3800 to the
 * shared cell at 0x801459F0 and calls the handler at 0x801F0BD8 indexed by
 * unsigned work byte 2 without a bounds check. Publishes 0x800D3800 to the
 * same cell, then appends another draw-mode packet using page 0x225 for
 * graphics types 1 or 2, otherwise 0x95. The pointer at 0x801F0BA0 names
 * this entry. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setDrawModeThenDispatchWorkByte2HandlerAndRestoreDrawModeBmagicMagic11303_801F0764(void) {
    s32 graph;

    if (GetGraphType() == 1 || GetGraphType() == 2) {
        graph = 0x2A5;
    } else {
        graph = 0xB5;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
    GpuAppendPrim(3, 0xC);
    D_801459F0 = 0x800C3800;
    D_801F0BD8[g_battle_work[2]]();
    D_801459F0 = 0x800D3800;
    if (GetGraphType() == 1 || GetGraphType() == 2) {
        graph = 0x225;
    } else {
        graph = 0x95;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
    GpuAppendPrim(3, 0xC);
}

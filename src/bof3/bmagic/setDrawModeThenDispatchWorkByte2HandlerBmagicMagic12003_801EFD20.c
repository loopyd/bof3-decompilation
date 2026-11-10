#include "bof3/bof3.h"
#include "gpu/prim.h"

extern s32 GetGraphType(void);
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern volatile u32 D_801459F0; /* @source 0x801459F0 @kind unknown */
extern void (*D_801F0CC4[])(void); /* @source 0x801F0CC4 @kind data */

/* @source 0x801EFD20
 * @behavior Selects texture page 0x25 when successive graphics-type queries
 *           return 1 or 2, otherwise 0x15. Sets a draw-mode packet at the
 *           primitive cursor with dfe 0, dtd 1 and no texture window, then
 *           appends its 0xC bytes under ordering-table slot 3. Publishes
 *           0x800C3800 to the shared cell at 0x801459F0, invokes the handler
 *           at 0x801F0CC4 indexed by unsigned work byte 2, and publishes
 *           0x800D3800 to the same cell. Entry 1 of the pointer run at
 *           0x801F0CA8; takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setDrawModeThenDispatchWorkByte2HandlerBmagicMagic12003_801EFD20(void) {
    s32 graph;

    if (GetGraphType() == 1 || GetGraphType() == 2) {
        graph = 0x25;
    } else {
        graph = 0x15;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
    GpuAppendPrim(3, 0xC);
    D_801459F0 = 0x800C3800;
    D_801F0CC4[g_battle_work[2]]();
    D_801459F0 = 0x800D3800;
}

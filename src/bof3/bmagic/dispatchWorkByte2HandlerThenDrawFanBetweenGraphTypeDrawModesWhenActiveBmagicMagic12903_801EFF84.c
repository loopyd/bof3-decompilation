#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void (*D_801F05CC[])(void); /* @source 0x801F05CC @kind data */
extern s32 GetGraphType(void); /* @source 0x8017B2B4 */
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size); /* @source 0x80155A08 */
extern void func_800AF764(void); /* @source 0x800AF764 */
extern void drawEightTranslucentGouraudFanTrianglesRadius32TintedByWorkByteATimesSignedBytes5d5e5fBmagicMagic12903_801F030C(void); /* @source 0x801F030C */

/* @source 0x801EFF84
 * @behavior Dispatches the unbounded work-byte-2 handler through the table at
 * 0x801F05CC, then re-reads the scratchpad work pointer. When work bytes 0 and
 * 2 are both nonzero, selects draw-mode page 0xA5 for graph types 1 or 2 and
 * 0x35 otherwise, sets and links a 12-byte draw-mode packet using work words
 * 0x34 and 0x38, calls the shared helper at 0x800AF764, and draws the eight
 * Gouraud fan triangles at 0x801F030C. Finally sets and links another draw-mode
 * packet with page 0x25 for graph types 1 or 2 and 0x15 otherwise. Each
 * graph-type test retains its separate call. Takes no arguments and returns
 * nothing; the work pointer is re-read after calls that can replace it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawFanBetweenGraphTypeDrawModesWhenActiveBmagicMagic12903_801EFF84(void) {
    s32 mode;

    D_801F05CC[g_battle_work[2]]();
    if (g_battle_work[0] != 0 && g_battle_work[2] != 0) {
        if (GetGraphType() == 1 || GetGraphType() == 2) {
            mode = 0xA5;
        } else {
            mode = 0x35;
        }
        SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, mode, 0);
        func_80155A08(FIELD_REF(s32, g_battle_work, 0x34),
                     FIELD_REF(s32, g_battle_work, 0x38), 2, 0xC);
        func_800AF764();
        drawEightTranslucentGouraudFanTrianglesRadius32TintedByWorkByteATimesSignedBytes5d5e5fBmagicMagic12903_801F030C();
        if (GetGraphType() == 1 || GetGraphType() == 2) {
            mode = 0x25;
        } else {
            mode = 0x15;
        }
        SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, mode, 0);
        func_80155A08(FIELD_REF(s32, g_battle_work, 0x34),
                     FIELD_REF(s32, g_battle_work, 0x38), 2, 0xC);
    }
}

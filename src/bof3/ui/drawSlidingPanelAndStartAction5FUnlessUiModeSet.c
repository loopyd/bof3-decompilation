#include "bof3/bof3.h"

/* Overlay-local panel step byte: it sits in the zero word immediately after
 * this overlay's handler run at 0x801D1750 and counts 0 through 4. */
extern u8 D_801D1774; /* @source 0x801D1774 @kind data */

/* CLUT bank byte the panel painters take as their trailing argument. */
extern u8 D_80144952; /* @source 0x80144952 @kind data */

/* Head mode/cursor byte of the shared UI work block; the commu work-block
 * initialiser 0x801D16A4 arms it to 2. */
extern u8 D_80148333; /* @source 0x80148333 @kind data */

/* Shared frontend mode byte the commit path latches. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Overlay step byte the enclosing handler run advances once its request
 * completes. */
extern u8 D_801F294C; /* @source 0x801F294C @kind data */

/* Shaded backdrop rectangle painter: x, y, width, height, shade and the CLUT
 * bank byte. Both leading coordinates are 16-bit, the width the same call sites
 * establish for this painter across the executable. */
extern void func_801AE3F0(u16 x, u16 y, u32 width, u32 height, u32 shade,
                          u32 clut); /* @source 0x801AE3F0 */

/* Inset rectangle painter used for the framed area inside a backdrop. */
extern void func_801AED54(s32 x, s32 y, s32 width, s32 height,
                          s32 arg4); /* @source 0x801AED54 */

/* Shared front-end frame/update slice. */
extern void func_80158E50(void); /* @source 0x80158E50 */

/* Shared action/entry starter taking an action id. */
extern void func_80150224(s32 action_id); /* @source 0x80150224 */

/* @source 0x801D11DC
 * @behavior Advances the overlay's panel step byte at 0x801D1774 while it is
 *           still below 4, then paints one framed panel at that step through
 *           the shaded backdrop painter func_801AE3F0, 0x118 by 0x13 at
 *           x = 0x14, and the inset painter func_801AED54, 0x113 by 0xF at
 *           x = 0x16. Both painters take the same y, 0x12 less twelve times the
 *           step, narrowed to the painter's 16-bit coordinate for the backdrop
 *           and taken unmodified by the 32-bit inset call, so the framed pair
 *           steps 12 pixels per count.
 *           With the shared UI work block still unarmed, its head mode/cursor
 *           byte at 0x80148333 reading zero, it also begins one shared
 *           front-end frame/update slice through func_80158E50, starts action
 *           0x5F through func_80150224, latches the shared frontend mode byte at
 *           0x80143BB0 to 2 and advances the overlay step byte at 0x801F294C by
 *           one. The advanced step value is read before the mode byte is
 *           latched and written after it. It takes no arguments and returns no
 *           value. Its only in-image reference is word 5 of this overlay's
 *           handler run based at 0x801D1758, which the step dispatcher
 *           0x801D0C5C indexes with the byte at 0x801F294D and calls with no
 *           arguments and no used result, so it has no jal caller.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSlidingPanelAndStartAction5FUnlessUiModeSet(void) {
    u8 step;

    if (D_801D1774 < 4) {
        D_801D1774 = D_801D1774 + 1;
    }
    func_801AE3F0(0x14, 0x12 - D_801D1774 * 12, 0x118, 0x13, 0xF2,
                  D_80144952);
    func_801AED54(0x16, 0x14 - D_801D1774 * 12, 0x113, 0xF, 0);
    if (D_80148333 != 0) {
        return;
    }
    func_80158E50();
    func_80150224(0x5F);
    step = D_801F294C;
    D_80143BB0 = 2;
    D_801F294C = step + 1;
}

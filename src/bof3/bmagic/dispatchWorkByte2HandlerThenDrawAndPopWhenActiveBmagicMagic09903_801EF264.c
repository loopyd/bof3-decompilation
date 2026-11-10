#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void (*D_801F09A0[])(void); /* @source 0x801F09A0 @kind data */

/* @source 0x800AF764 */
extern void func_800AF764(void);
/* @source 0x801EF1B0 */
extern void func_801EF1B0(void);
/* @source 0x801EF8F0 */
extern void func_801EF8F0(s32, s32, s32);

/* @source 0x801EF264
 * @behavior Call the handler indexed by work byte 2 in the table at
 *           0x801F09A0, then call the shared helper at 0x800AF764.
 *           Re-read the scratchpad work pointer and, if byte 2 is nonzero,
 *           install the work transform through 0x801EF1B0, draw through
 *           0x801EF8F0 with arguments (0x60, 0x20, 7), and PopMatrix.
 *           The overlay's byte-1 handler table publishes this callback.
 *           No arguments or return value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawAndPopWhenActiveBmagicMagic09903_801EF264(void) {
    D_801F09A0[g_battle_work[2]]();
    func_800AF764();
    if (g_battle_work[2] != 0) {
        func_801EF1B0();
        func_801EF8F0(0x60, 0x20, 7);
        PopMatrix();
    }
}

#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void (*D_801F09C0[])(void); /* @source 0x801F09C0 @kind data */

/* @source 0x800AF764 */
extern void func_800AF764(void);
/* @source 0x801EF1B0 */
extern void func_801EF1B0(void);
/* @source 0x801F043C */
extern void func_801F043C(void);
/* @source 0x801F070C */
extern void func_801F070C(void);

/* @source 0x801F0008
 * @behavior Call the handler indexed by work byte 2 in the table at
 *           0x801F09C0, then call the shared helper at 0x800AF764.
 *           Re-read the scratchpad work pointer and, if byte 2 is nonzero,
 *           call 0x801EF1B0, 0x801F043C, and 0x801F070C, then PopMatrix.
 *           The overlay publishes this callback in its table at 0x801F0994.
 *           No arguments or return value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenCall043CAnd070CAndPopWhenActiveBmagicMagic09903_801F0008(void) {
    D_801F09C0[g_battle_work[2]]();
    func_800AF764();
    if (g_battle_work[2] != 0) {
        func_801EF1B0();
        func_801F043C();
        func_801F070C();
        PopMatrix();
    }
}

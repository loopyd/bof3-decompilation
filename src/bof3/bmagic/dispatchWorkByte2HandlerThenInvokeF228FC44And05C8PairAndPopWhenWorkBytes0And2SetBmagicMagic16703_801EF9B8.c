#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void (*D_801F08C4[])(void); /* @source 0x801F08C4 @kind data */

extern void func_801EF228(void); /* @source 0x801EF228 @kind function */
extern void func_801EFC44(void); /* @source 0x801EFC44 @kind function */
extern void func_801F05C8(s32 mode); /* @source 0x801F05C8 @kind function */

/* @source 0x801EF9B8
 * @behavior Dispatches through the handler table at 0x801F08C4 using unsigned
 *           work byte 2 from the scratchpad pointer cell at 0x1F800044.
 *           Reloads the pointer after the handler and tests work bytes 0 and 2
 *           in that order. When both are nonzero, invokes 0x801EF228, reloads
 *           the pointer, and invokes 0x801EFC44 only when work byte 2 is below
 *           three. Then invokes 0x801F05C8 with arguments two and three and
 *           restores the matrix stack with PopMatrix. The table word at
 *           0x801F08B4 names this callback. Takes no arguments and returns
 *           nothing; the dispatch index is not bounded.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeF228FC44And05C8PairAndPopWhenWorkBytes0And2SetBmagicMagic16703_801EF9B8(void) {
    D_801F08C4[g_battle_work[2]]();
    if (g_battle_work[0] != 0 && g_battle_work[2] != 0) {
        func_801EF228();
        if (g_battle_work[2] < 3) {
            func_801EFC44();
        }
        func_801F05C8(2);
        func_801F05C8(3);
        PopMatrix();
    }
}

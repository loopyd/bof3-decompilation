#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void (*D_801F07D4[])(void); /* @source 0x801F07D4 @kind data */
extern void func_801EF128(void); /* @source 0x801EF128 @kind function */
extern void func_801F04E0(void); /* @source 0x801F04E0 @kind function */

/* @source 0x801F02BC
 * @behavior Dispatches through the table at 0x801F07D4 using unsigned work
 *           byte 2 from the pointer at 0x1F800044, without bounding the index.
 *           Re-reads that pointer after the handler and tests work bytes 0
 *           and 2 in that order. When both are nonzero, calls the local
 *           helpers at 0x801EF128 and 0x801F04E0, then PopMatrix. Takes no
 *           arguments and returns nothing. Helper roles remain address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenCallF128And04E0AndPopWhenWorkBytes0And2SetBmagicMagic10303_801F02BC(void) {
    D_801F07D4[g_battle_work[2]]();
    if (g_battle_work[0] != 0 && g_battle_work[2] != 0) {
        func_801EF128();
        func_801F04E0();
        PopMatrix();
    }
}

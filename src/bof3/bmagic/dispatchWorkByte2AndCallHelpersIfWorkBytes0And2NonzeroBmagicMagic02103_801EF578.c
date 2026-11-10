#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void (*D_801EFE08[])(void); /* @source 0x801EFE08 @kind data */
extern void func_800AF764(void); /* @source 0x800AF764 @kind function */
extern void func_801EF7FC(void); /* @source 0x801EF7FC @kind function */
extern void func_801EFA48(void); /* @source 0x801EFA48 @kind function */

/* @source 0x801EF578
 * @behavior Calls the handler indexed by unsigned work byte 2 in the local
 * table at 0x801EFE08. Reloads the work pointer after the call; when work
 * bytes 0 and 2 are both nonzero, calls helpers at 0x800AF764, 0x801EF7FC
 * and 0x801EFA48 in that order. Takes no arguments and returns nothing.
 * The pointer cell is at scratchpad 0x1F800044; byte roles remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2AndCallHelpersIfWorkBytes0And2NonzeroBmagicMagic02103_801EF578(void) {
    u8 *work;

    D_801EFE08[g_battle_work[2]]();
    work = g_battle_work;
    if (work[0] != 0 && work[2] != 0) {
        func_800AF764();
        func_801EF7FC();
        func_801EFA48();
    }
}

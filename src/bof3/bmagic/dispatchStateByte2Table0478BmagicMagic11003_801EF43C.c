#include "bof3/bof3.h"

/* Interior word of the handler run at 0x801F046C. Work byte 2 selects
 * entries starting at 0x801F0478 without an index bound. */
extern void (*D_801F0478[])(void); /* @source 0x801F0478 @kind data */

/* Shared dispatch-context cell published around the handler and service calls. */
extern volatile u32 D_801459F0; /* @source 0x801459F0 @kind unknown */

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* This call site passes no arguments to the shared main-executable service. */
extern void func_8014D290(void); /* @source 0x8014D290 @kind function */

/* @source 0x801EF43C
 * @behavior Entry 0 of the byte-1 handler run at 0x801F046C. Publishes
 *           0x800C3800 into the shared cell at 0x801459F0 and calls the
 *           handler at 0x801F0478 indexed by unsigned work byte 2. Re-reads
 *           the scratchpad work pointer at 0x1F800044 after the handler.
 *           When work byte 0 has bit 0 set and work byte 2 is at least 2,
 *           calls the shared service at 0x8014D290. Finally publishes
 *           0x800D3800 into 0x801459F0. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2Table0478BmagicMagic11003_801EF43C(void) {
    u8 *work;

    D_801459F0 = 0x800C3800;
    D_801F0478[g_battle_work[2]]();
    work = g_battle_work;
    if ((work[0] & 1) != 0 && work[2] >= 2) {
        func_8014D290();
    }
    D_801459F0 = 0x800D3800;
}

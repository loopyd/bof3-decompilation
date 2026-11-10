#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void (*D_801F0704[])(void); /* @source 0x801F0704 @kind data */

/* @source 0x801F0284 */
extern void func_801F0284(void);
/* @source 0x801F0334 */
extern void func_801F0334(void);

/* @source 0x801EFFA4
 * @behavior Dispatches the handler indexed by unsigned work byte 2 from the
 * table at 0x801F0704 without a bounds check. Re-reads the scratchpad work
 * pointer after the handler; when work bytes 0 and 2 are both nonzero, calls
 * 0x801F0284, then 0x801F0334, then PopMatrix. The work pointer cell is at
 * 0x1F800044. The table word at 0x801F0700 names this callback. It takes no
 * arguments, returns nothing, and has no direct object stores.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvoke0284And0334AndPopWhenWorkBytes0And2SetBmagicMagic22003_801EFFA4(void) {
    u8 *step_record;
    u8 *draw_record;

    step_record = g_battle_work;
    D_801F0704[step_record[2]]();
    draw_record = g_battle_work;
    if (draw_record[0] != 0 && draw_record[2] != 0) {
        func_801F0284();
        func_801F0334();
        PopMatrix();
    }
}

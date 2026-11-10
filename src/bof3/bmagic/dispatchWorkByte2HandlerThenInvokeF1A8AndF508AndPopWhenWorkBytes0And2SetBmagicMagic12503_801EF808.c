#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void (*D_801F05EC[])(void); /* @source 0x801F05EC @kind data */

/* @source 0x801EF1A8 */
extern void func_801EF1A8(void);
/* @source 0x801EF508 */
extern void func_801EF508(void);

/* @source 0x801EF808
 * @behavior Dispatches the handler indexed by unsigned work byte 2 from the
 * table at 0x801F05EC without a bounds check. Re-reads the scratchpad work
 * pointer after the handler; when work bytes 0 and 2 are both nonzero, calls
 * 0x801EF1A8, then 0x801EF508, then PopMatrix. The work pointer cell is at
 * 0x1F800044. The table word at 0x801F05DC names this callback. It takes no
 * arguments, returns nothing, and has no direct object stores.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeF1A8AndF508AndPopWhenWorkBytes0And2SetBmagicMagic12503_801EF808(void) {
    u8 *step_record;
    u8 *draw_record;

    step_record = g_battle_work;
    D_801F05EC[step_record[2]]();
    draw_record = g_battle_work;
    if (draw_record[0] != 0 && draw_record[2] != 0) {
        func_801EF1A8();
        func_801EF508();
        PopMatrix();
    }
}

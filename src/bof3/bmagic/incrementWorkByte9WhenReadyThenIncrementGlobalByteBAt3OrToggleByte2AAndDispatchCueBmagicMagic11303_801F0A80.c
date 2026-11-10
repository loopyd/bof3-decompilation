#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern u8 func_8014DAEC(void); /* @source 0x8014DAEC @kind function */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */
extern void func_8014D6B8(u32 selector); /* @source 0x8014D6B8 @kind function */
extern void func_8014D290(void); /* @source 0x8014D290 @kind function */
extern void func_8015DF18(u16 cue); /* @source 0x8015DF18 @kind function */

/* @source 0x801F0A80
 * @behavior Calls readiness helper 0x8014DAEC. On a nonzero byte result,
 * increments work byte 9 modulo 256. If the new byte is at least 3,
 * increments shared battle work byte B and calls 0x801E5988. Otherwise
 * toggles work byte 0x2A between 0 and 1, calls 0x8014D6B8 with work byte B,
 * and calls 0x8014D290. When the reloaded work byte B is zero, dispatches
 * cue 0x104 if byte 9 is 1, or cue 0x100 otherwise. A zero readiness result
 * calls only 0x8014D290. Takes no arguments and returns nothing. Table word
 * 0x801F0BE0 names this handler; unknown helper and field roles remain
 * address- or offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9WhenReadyThenIncrementGlobalByteBAt3OrToggleByte2AAndDispatchCueBmagicMagic11303_801F0A80(void) {
    u8 *work;
    u8 *globalWork;
    u8 *toggle;
    u8 *cueWork;

    if (func_8014DAEC() != 0) {
        work = g_battle_work;
        work[9]++;
        if (work[9] >= 3) {
            globalWork = D_801EB4E0;
            globalWork[0xB]++;
            func_801E5988();
        } else {
            toggle = g_battle_work;
            toggle[0x2A] = (toggle[0x2A] + 1) & 1;
            func_8014D6B8(g_battle_work[0xB]);
            func_8014D290();
            cueWork = g_battle_work;
            if (cueWork[0xB] == 0) {
                if (cueWork[9] == 1) {
                    func_8015DF18(0x104);
                } else {
                    func_8015DF18(0x100);
                }
            }
        }
    } else {
        func_8014D290();
    }
}

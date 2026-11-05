#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146867;
extern u16 D_80149328;
/* Both writes to this shared counter are observable in the original. */
extern volatile u16 D_8014932A;
extern u8 D_80149333;

/* @source 0x801FCBDC
 * @behavior Alternates phases 0 and 1 in D_80146865, advancing the byte
 * D_80146867 modulo 256 and resetting it when it reaches two. Each active
 * phase publishes mode 2 at D_80149333, writes counter 0x8014932A plus
 * (phase 0) or minus (phase 1) the unsigned byte step, then overwrites that
 * same counter with 0x80149328 plus the step. On the second tick it clears
 * both halfword counters, republishes mode 2 and toggles the phase. Other
 * phase values do nothing. The sole caller passes 10 and ignores the return.
 * @status partial
 * @match 48.72
 * @residual Entry pointer/copy and register allocation differ; 38/78 instructions match.
 */
void alternateTwoTickCounterPhasesScenarioScena0200_801FCBDC(u8 step) {
    u8 tick;
    u16 next2;
    u16 next1;

    switch (D_80146865) {
    case 0:
        tick = D_80146867;
        next2 = D_8014932A;
        next1 = D_80149328;
        D_80149333 = 2;
        tick++;
        next2 += step;
        next1 += step;
        D_80146867 = tick;
        D_8014932A = next2;
        D_8014932A = next1;
        if (tick == 2) {
            D_8014932A = 0;
            D_80149328 = 0;
            D_80149333 = 2;
            D_80146865 = 1;
            D_80146867 = 0;
        }
        break;
    case 1:
        tick = D_80146867;
        next2 = D_8014932A;
        next1 = D_80149328;
        D_80149333 = 2;
        tick++;
        next2 -= step;
        next1 += step;
        D_80146867 = tick;
        D_8014932A = next2;
        D_8014932A = next1;
        if (tick == 2) {
            D_8014932A = 0;
            D_80149328 = 0;
            D_80149333 = 2;
            D_80146867 = 0;
            D_80146865 = 0;
        }
        break;
    }
}

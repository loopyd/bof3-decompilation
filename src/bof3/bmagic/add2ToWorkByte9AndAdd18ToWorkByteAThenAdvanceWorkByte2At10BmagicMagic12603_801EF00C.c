#include "bof3/bof3.h"

extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801EF00C
 * @behavior Adds two to scratchpad work-object byte 9 modulo 256, adds 0x18
 *           to byte 0xA modulo 256, then compares byte 9 against 0x10 and
 *           increments work byte 2 on equality. Each of the three stores in
 *           the original re-reads the work pointer cell at 0x1F800044, so the
 *           body keeps one lui/lw pair per statement, and the guarded tail
 *           shares the base its own test loaded. Word 4 of the pointer run
 *           based at 0x801F125C names this address (0x801F126C), so the body
 *           runs as a battle work-step handler and the payload holds no jal
 *           caller for it. Takes no arguments, returns nothing, keeps no
 *           frame and touches no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add2ToWorkByte9AndAdd18ToWorkByteAThenAdvanceWorkByte2At10BmagicMagic12603_801EF00C(void) {
    D_1F800044[9] += 2;
    D_1F800044[0xA] += 0x18;
    if (D_1F800044[9] == 0x10) {
        D_1F800044[2]++;
    }
}

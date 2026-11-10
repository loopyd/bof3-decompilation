#include "bof3/bof3.h"

extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */
extern u8 D_801F294D; /* @source 0x801F294D @kind data */
extern u8 D_801D1774; /* @source 0x801D1774 @kind data */
extern void func_80158E50(void); /* @source 0x80158E50 */

/* Commu overlay's own UI work-block initialiser at 0x801D16A4, compiled from
 * this target's own source, so it needs a prototype and no binding. */
void initializeCommuUiState(void);

/* @source 0x801D0CF4
 * @behavior Reads the shared frontend mode byte at 0x80143BB0 and returns while
 *           it already equals 2. Otherwise it begins one shared front-end
 *           frame/update slice through func_80158E50, initialises the commu
 *           overlay's UI work block through initializeCommuUiState at
 *           0x801D16A4, stores 4 into the state byte at 0x801D1774 and advances
 *           the overlay step byte at 0x801F294D by one, returning no value. The
 *           step byte was loaded before the state byte was armed and both byte
 *           stores followed that load, so the local carries it. It takes no
 *           arguments and reads no other state. Its only in-image reference is
 *           word 1 of this overlay's handler run based at 0x801D1758, which the
 *           step dispatcher 0x801D0C5C indexes with the byte at 0x801F294D and
 *           calls with no arguments and no used result, so it has no jal caller.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeCommuUiAndArmStateByteUnlessFrontendMode2(void) {
    u8 step;

    if (D_80143BB0 == 2) {
        return;
    }
    func_80158E50();
    initializeCommuUiState();
    step = D_801F294D;
    D_801D1774 = 4;
    D_801F294D = step + 1;
}

#include "bof3/bof3.h"

extern u8 D_801462E3;
extern u8 D_801462E4;
extern u8 D_801462E5;

/* @source 0x801EEFCC
 * @behavior Advances the battle handler-run selector once the EXE-side EMI
 *           loader reports ready: clears the battle state byte at 0x801462E5
 *           (g_BtlState in include/battle/ram.h) and the run-local substate
 *           index at 0x801462E4, then increments the selection byte at
 *           0x801462E3 that dispatches this overlay's handler runs. The
 *           selector load is scheduled ahead of both clears. Takes no
 *           arguments and returns nothing; the 0x18-byte frame only keeps $ra
 *           across the loader poll.
 * @status exact
 * @match 100.00
 * @residual none
 * 0x801EEFCC is the last entry (index 4) of the handler run based at
 * 0x801F0CA4, so this body is what moves the selection byte on to the next run
 * once that run's handlers are done. D_801462E3 is read as the index of the
 * table at 0x801F0C98 by dispatchSubstateTable0c98BatlEnd00_801EEC58, and
 * D_801462E4 is the index the 0x801F0CA4 dispatcher scales into that run.
 */
void advanceHandlerRunWhenLoaderReadyBatlEnd00_801EEFCC(void) {
  if (func_80162D00() != 0) {
    D_801462E5 = 0;
    D_801462E4 = 0;
    D_801462E3 += 1;
  }
}

#include "bof3/bof3.h"

/* Shared UI sub-step byte of the main executable, the cell this overlay's
 * sub-step dispatcher D_801D20AC indexes its handler table with and that the
 * handler at 0x801D211C clears once the front-end effect gate is idle. It lies
 * outside this EMI payload and is read and written at the original unsigned
 * byte width.
 * @source 0x80148652 @kind data
 */
extern u8 D_80148652;

/* @source 0x801D20E8
 * @behavior First handler of the BATE overlay sub-step handler pointer table
 *           D_801D8770, reached as its entry zero. It requests frontend-local
 *           mode 3 through the main-exe setter func_8014ECAC and then advances
 *           the shared UI sub-step byte D_80148652 by one, so control leaves
 *           this handler in another sub-step and still in the front-end input
 *           mode that call installs. The 0x18-byte frame only saves the return
 *           address at 0x10 across that call; the setter is called before the
 *           increment, the address of the sub-step byte is materialized once
 *           after that call and reused for both the load and the store, and
 *           nothing is returned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setFrontLocalMode3AndAdvanceUiSubstep(void) {
  u8 *cell;

  func_8014ECAC(3);
  cell = &D_80148652;
  *cell += 1;
}

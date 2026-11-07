#include "bof3/bof3.h"

/* Shared scenario progress byte at 0x80146864, bound by the shared map. This
 * step runs only once the progress byte has reached 0x1C. The body reads it
 * through the plain symbol expression, which is the `lui $v1,%hi` plus folded
 * `lbu $v1,%lo` pair the original bytes show. */
extern u8 g_ScenarioProgress;

/* Shared overlay state byte at 0x8014933B, the byte the previous handler
 * 0x801F405C raises to 1 and this step clears again. The store re-materialises
 * `$at` for its own `lui`/`sb` pair. */
extern u8 D_8014933B; /* @source 0x8014933B @kind data */

/* Shared work-flag reset helper at 0x80196070, the address mapped as
 * clearWorkFlags in emi/etc/game/00. It sits outside this overlay's payload, so
 * only its prototype is declared. It takes no arguments, returns nothing and
 * zeroes work-object bytes 0x00-0x04, which also puts the game work area's
 * handler index back to zero. */
void func_80196070(void);

/* @source 0x801F4128
 * @behavior Entry 3 of the four-pointer handler table at 0x801F5718 that the
 *           byte-+0x02 dispatcher 0x801F3FF8 indexes with work-record byte +0x02
 *           of the record published at scratchpad pointer cell 0x1F800044.
 *           Terminal cleanup step of the overlay's handler sequence: while the
 *           shared scenario progress byte at 0x80146864 reads 0x1C it clears the
 *           shared overlay state byte at 0x8014933B that the preceding handler
 *           0x801F405C had raised, then clears the shared work flags through the
 *           helper at 0x80196070 (clearWorkFlags in emi/etc/game/00); every
 *           other progress value leaves both the state byte and the work flags
 *           untouched. The progress byte is read once through its symbol
 *           expression, the state byte store carries its own `$at` base, and the
 *           0x18-byte frame keeps the return address at +0x10 across the single
 *           call whose argumentless `jal` keeps the `nop` delay slot. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSharedStateByteThenClearWorkFlagsAtProgress1cWorld02Area10814_801F4128(void) {
  if (g_ScenarioProgress == 0x1C) {
    D_8014933B = 0;
    func_80196070();
  }
}

#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 pad-mask wrapper selected by the overlay's handler pointer
 * table: calls the target-local gate func_801DDF7C with the shared pad mask
 * 0x50, discards the gate result, then calls the shared helper func_8014D978
 * without arguments and returns.
 * @source 0x801DC568
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DC568(void) {
    func_801DDF7C(0x50);
    func_8014D978();
}

#include "internal.h"

/* @source 0x801E4DD8
 * @behavior Calls func_801E3160 and increments scratchpad work byte +0x02.
 */
void func_801E4DD8(void) {
    func_801E3160();
    D_1F800044->unk_02++;
}

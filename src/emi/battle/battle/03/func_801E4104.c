#include "internal.h"

/* @source 0x801E4104
 * @behavior Increments scratchpad byte +0x03 when func_801E3160 returns nonzero.
 */
void func_801E4104(void) {
    if (func_801E3160() != 0) {
        SPAD_PTR_SLOT(u8, 0x44)[3]++;
    }
}

#include "internal.h"

/* @source 0x80096AB0
 * NOTE: byte-match blocked by the `jr ra` epilogue scheduler reorg: the repo
 * build uses default `-fschedule-insns`, which hoists the `sb` cell-store past
 * the `lw ra` load. The original battle/15 object was built with
 * `-fno-schedule-insns` (program-order scheduling); restoring that per-target
 * profile makes this match. CLOBBER_*/barrier() do not apply (they govern
 * jal/branch delay slots, not the epilogue).
 */
void func_80096AB0(void) {
    func_801DE94C(0, 0);
    PSX_REF(volatile u8, (u32)&D_801462E3) += 1;
}

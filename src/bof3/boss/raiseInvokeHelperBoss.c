#include "bof3/bof3.h"

void func_801E5988(void);

extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x800C1CFC
 * @behavior Step handler that raises bit 0x4 of the shared battle global
 * halfword at 0x801462E8 with a halfword read-modify-write and then invokes
 * the shared helper at 0x801E5988 with no arguments, unconditionally. The
 * halfword address is materialized once into a register that carries the load,
 * the ori and the store, and the store fills the call delay slot. This address
 * is entry 9 of the thirteen-word code pointer run at 0x800C1804, held by the
 * word at 0x800C1824 and the last of the ten entries that
 * callStateTableFrame copies into its frame and
 * indexes by work record byte 1, so no jal and no other word of the payload
 * names it. The 0x18-byte frame only keeps $ra at 0x10 across the helper call;
 * nothing is returned and no other state is read or written.
 * @status partial
 * @match 85.71
 * @residual Preserving volatile access keeps the halfword store before the helper call instead of its delay slot, adding one instruction.
 */
void raiseInvokeHelperBoss(void) {
  volatile u16 *flags = &D_801462E8;

  *flags |= 4;
  func_801E5988();
}

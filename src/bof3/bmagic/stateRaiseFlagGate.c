#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind
 * the same word (BATTLE_GLOBAL_HALF_62E8); it lives outside this overlay, so it
 * keeps its address-qualified name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* Guard byte at 0x801EC2E4. The battle overlay lifts read and write the same
 * word as a plain byte and establish no role for it, so it keeps its
 * address-qualified name. */
extern u8 D_801EC2E4; /* @source 0x801EC2E4 @kind unknown */

/* @source 0x801EEF58
 * @behavior Reads the shared battle byte at 0x801EC2E4 with an unsigned byte
 * load before the 0x18-byte frame is created and, only when that byte is zero,
 * raises bit 0x4 of the shared battle global halfword at 0x801462E8 with an
 * unsigned halfword read-modify-write and then invokes the shared helper
 * func_801E5988. The halfword address stays in one register across the
 * read-modify-write, so it is written through a local pointer, and the frame
 * only keeps $ra across the helper call. It takes no arguments, returns
 * nothing and writes no other state. Its address is the third word (index 2)
 * of the three-entry code pointer table at 0x801EEC04, whose words are
 * 0x801EEC70, 0x801EECDC and 0x801EEF58 and which the overlay entry dispatches
 * on unsigned work byte 1.
 * @status partial
 * @match 82.35
 * @residual Preserving volatile access keeps the halfword store before the helper call instead of its delay slot, adding one instruction and moving the guard target.
 */
void stateRaiseFlagGate(void) {
  if (D_801EC2E4 == 0) {
    volatile u16 *flags = &D_801462E8;

    *flags |= 4;
    func_801E5988();
  }
}

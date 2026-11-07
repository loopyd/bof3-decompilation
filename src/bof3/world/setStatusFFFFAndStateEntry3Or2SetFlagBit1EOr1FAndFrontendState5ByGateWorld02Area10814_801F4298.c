#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern s8 D_801448EB;
extern u8 D_801448ED;

void func_8015B580(void *bitmap, s32 bit_index);

/* @source 0x801F4298
 * @behavior Overlay state setter, entry 0 of the in-image code-pointer run based
 *           at 0x801F62DC in the overlay's reviewed textbin data, so no in-image
 *           instruction references it and the overlay is entered from outside its
 *           payload. It reads the shared gate byte at 0x801490C7 with a signed
 *           byte load (`lb`, so the cell is s8), stores 0xFFFF into the shared
 *           status halfword at 0x801490A8 on every path, and then switches on the
 *           gate byte. Each handled arm takes the shared state-entry byte
 *           0x801448ED's address once (`lui`/`addiu` into `$a0`, then `sb 0($a0)`),
 *           writes 3 for gate 0 or 2 for gate 1, advances that same base by 0x63B
 *           to the shared flag block at 0x80144F28 and sets flag bit 0x1E or 0x1F
 *           of it through the shared bit-set helper at 0x8015B580. The two arms'
 *           `jal` share one site at the join because they differ only in the
 *           pre-loaded argument registers, while the state-byte store and the
 *           address advance stay in each arm because their bases are separate
 *           values. Gate 1's state constant fills the delay slot of its own test
 *           and the `jal` delay slot stays unfilled. Any other gate value returns
 *           before the shared frontend state byte write. Only the two handled gate
 *           values reach the closing store of 5 into the shared frontend state
 *           byte 0x801448EB, the front-end selector that the front-end dispatcher
 *           0x801A7CAC indexes its handler table with. Takes no arguments and
 *           returns nothing; the 0x18-byte frame only keeps $ra.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndStateEntry3Or2SetFlagBit1EOr1FAndFrontendState5ByGateWorld02Area10814_801F4298(
    void) {
  s32 gate;
  u8 *entry;

  gate = D_801490C7;
  D_801490A8 = 0xFFFF;
  switch (gate) {
  case 0:
    entry = &D_801448ED;
    *entry = 3;
    func_8015B580(entry + 0x63B, 0x1E);
    break;
  case 1:
    entry = &D_801448ED;
    *entry = 2;
    func_8015B580(entry + 0x63B, 0x1F);
    break;
  default:
    return;
  }
  D_801448EB = 5;
}

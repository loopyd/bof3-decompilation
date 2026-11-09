#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801448EB;
extern u8 D_801448EC;
extern u8 D_801448ED;

void func_8015C088(void);

/* @source 0x801F4618
 * @behavior Stores 0xFFFF into the shared status halfword at 0x801490A8 on
 *           every path (`ori v0,0,0xFFFF`, `sh`) and, while the shared gate byte
 *           at 0x801490C7 is zero (`lb`, so the cell is s8), runs the shared
 *           front-end startup helper func_8015C088 and then writes the shared
 *           state-entry tuple: 0x801448EB = 0x2B from one `addiu v0,0,0x2B`, then
 *           0x801448EC = 0 and 0x801448ED = 0, which both store `$zero`. The
 *           gate load is hoisted above the status store, `bnez` jumps to the
 *           epilogue so the gate-clear path falls through its three byte stores
 *           into the frame restore, and both the branch delay slot and the call
 *           delay slot stay nops. The 0x18-byte frame only keeps $ra across the
 *           helper call, and each store materialises the 0x8014 page in $at on
 *           its own. Takes no arguments and returns nothing.
 *
 * Name evidence: 0x801448EB is the state-entry byte of the shared tuple the
 * setStatusFFFFAndStateEntry* handlers write, with 0x801448EC and 0x801448ED as
 * its progress and substate companions, 0x801490C7 is the shared gate byte those
 * handlers branch on, and 0x8015C088 is the shared front-end startup helper the
 * whole world family runs. The payload names this body only from a table word:
 * 0x801F4DFC holds the area handler pointers, so no in-image `jal` reaches
 * 0x801F4618 and its roles stay structural. The entry digit follows the family's
 * hex spelling, so 2B here is 0x2B.
 *
 * Shape: statement order, operand forms, offset spelling and declaration style
 * are taken from setStatusFFFFAndStateEntry25Progress5WhenGateClear
 * World04Area17013_801F4950, whose 0x54-byte body differs from this 0x58-byte
 * span only by storing a non-zero progress in 0x801448EC and one more byte, so
 * that body is shape evidence rather than a clone. The nearest in-target
 * sibling, func_801F4670 at 0x801F4670, writes the same 0x801448EB = 0x2B with
 * 0x801448EC = 0 and sets 0x801448ED = 1, differing from this span in that single
 * constant, which is why the two zero stores here spell `$zero` directly. No
 * byte-identical twin exists: comparing the mnemonic/operand shape of this
 * 0x58-byte body against every other 0x2B-storing `func_8015C088` handler under
 * out/splat matched only itself.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndStateEntry2BWhenGateClearWorld04Area18813_801F4618(void) {
  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 0) {
    func_8015C088();
    D_801448EB = 0x2B;
    D_801448EC = 0;
    D_801448ED = 0;
  }
}

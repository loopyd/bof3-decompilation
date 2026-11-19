#include "bof3/bof3.h"

/* Base of the shared main-RAM flag bank the EXE-side setter below indexes:
 * this body materializes it as `lui $a0,0x8014` / `addiu $a0,$a0,0x4F28`. */
extern u8 D_80144F28; /* @source 0x80144F28 @kind unknown */

void func_8015B580(void* arg0, s32 arg1);

/* @source 0x801F56E0
 * @behavior Overlay flag raiser of the byte-2 state machine. It materializes the
 *           shared main-RAM flag bank D_80144F28 as a `lui`/`addiu` $a0 pair and
 *           sets flag 0x67 through the EXE-side flag setter func_8015B580, with
 *           0x67 in the `jal` delay slot. The in-image consumer is the byte-2
 *           state handler at 0x801F4574 of this same overlay, which tests the
 *           same flag of the same bank through func_8015B5D4 and advances the
 *           record state byte once the flag is set, so this body arms the
 *           condition that handler waits on. Takes no arguments, returns
 *           nothing, reads no other state, and the 0x18-byte frame only keeps
 *           ra at 0x10 across the setter call.
 *
 * Name evidence: the setter call materializes the bank as the
 * `lui/addiu $a0,%hi/%lo(D_80144F28)` pair with `addiu $a1,$zero,0x67` in its
 * delay slot, so 0x67 is the flag index this body arms; the consumer at
 * 0x801F4574 reads the identical bank and index. No role beyond "the flag the
 * byte-2 advance handler tests" is evidenced, so the name records the write
 * rather than inventing a game-state role.
 *
 * Shape evidence: statement order, operand forms and declaration style are
 * taken from the in-tree setter callers setSharedFlagBit4World02Area07714_801F5BD8
 * and setSharedFlag23WhenSharedSelector5IsSetWorld02Area09713_801F2D28, which
 * carry the same `func_8015B580(&D_80144F28, <flag>)` body. They are shape
 * evidence rather than clone sources: this 0x28-byte span occurs byte-identically
 * only at 0x2AE0 of this payload across the payloads under out/binaries, so the
 * clone lever does not apply, and the only difference from those siblings is the
 * flag immediate, which is written here from the original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSharedFlag67World04Area17013_801F56E0(void) {
  func_8015B580(&D_80144F28, 0x67);
}

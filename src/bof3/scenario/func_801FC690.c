#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FC690
 * @behavior Slot 7 of the overlay's nine-pointer handler table at 0x801FD00C
 * (D_801FD00C), which the record dispatcher func_801FC46C selects with byte
 * 0x7A of its record argument and calls as (record, D_8014686C): it runs the
 * shared front-end startup helper func_8015C088 and then requests primary scene
 * state 6 with a cleared secondary sub-state by storing 6 in the shared state
 * byte D_80146874 and 0 in the shared sub-state byte D_80146875, and finally
 * returns 0 for the caller. The record pointer in $a0 and the shared scenario
 * flag word in $a1 are never read; the 0x18-byte frame only keeps $ra across
 * the call, and each of the two byte stores materialises the 0x8014 page in
 * $at on its own, which is how the psyq compiler emits separate shared-byte
 * stores.
 * @status partial
 * @match 84.62
 * @residual Post-call constant store materialises 6 in $v0 where the shipped bytes use $v1: insn=11/13, 52->52 bytes, frame 0x18 unchanged, first difference +0x0010 (li v1,6 vs li v0,6).
 * Four clean-C spellings measured live this lane all reproduced $v0 at that same first difference: a result local assigned after the stores, the same local assigned before them, a named state local feeding the store, and a state+result local pair.
 * The identical residual class is already recorded on src/bof3/ui/func_801F14F8_commu00.c (@status partial, @match 87.50), whose lane measured eight further spellings with the same outcome; residual class: reload/allocator register choice for a post-call constant store, which the measured clean-C type/control-flow/expression spellings do not move.
 * Next evidence: an authorised compiler-profile probe or a scheduler-order route; independent review pending.
 */
s32 func_801FC690(s32 arg0, s32 arg1) {
  func_8015C088();
  D_80146874 = 6;
  D_80146875 = 0;
  return 0;
}

#include "bof3/bof3.h"

extern u16 D_80143F00;

s32 func_801F9DF4(s32 arg0, s32 arg1);

/* @source 0x801F9DB4
 * @behavior Route-0x28 gate of this overlay in front of its own packed-record
 * handler func_801F9DF4 (0x801F9DF4): when the shared route word D_80143F00
 * reads 0x28 (40) it forwards its own two incoming packed record words
 * unchanged (no argument register is set up anywhere in its body) and returns
 * the sign-extended low byte of that handler's result (the `sll`/`sra` by 24 at
 * 0x801F9DDC), so it reports 1 only on that path; for every other route value it
 * reports 0, materialised as `addu $v0,$zero,$zero` in the jump delay slot of
 * the branch to the shared epilogue at 0x801F9DE4. The 0x18-byte frame only
 * keeps $ra across the call, and that store is sunk into the `beq` delay slot at
 * 0x801F9DC8. It reads no other state and writes none. No in-image jal targets
 * this address: its only image word is 0x801F9DB4 at 0x801FA1F8, word 5 of the
 * eight-word code-pointer run at 0x801FA1E4 (0x801F7AC8, 0x801F7B00, 0x801F7BC0,
 * 0x801F8124, 0x801F9D48, 0x801F9DB4, 0x801F9DAC, 0x801FA010) that the work
 * byte 1 dispatcher dispatchWorkObjectHandlerScenarioScena0400_801F7A84 indexes
 * with no arguments of its own.
 *
 * Name evidence: the body is the exact instruction shape of the exact lift
 * forwardRoute47RecordScenarioScena0300_801FCCDC (0x801FCCDC), whose 64 bytes
 * are identical here except for the carried route constant (0x47 there, 0x28 in
 * this span's own `addiu` at 0x801F9DC0) and its two absolute words, the branch
 * to the shared epilogue and the call, each shifted by the 0x2F28 span delta; so
 * this function takes that sibling's role and name wording with its own
 * constant. The route-word role of D_80143F00 is the accepted form of
 * dispatchRouteHandlerScenarioScena0300_801FC704 and forwardRoute47Record, and
 * the packed-word forwarding spelling is theirs as well.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 forwardRoute28RecordScenarioScena0400_801F9DB4(s32 arg0, s32 arg1) {
  if (D_80143F00 == 0x28) {
    return (s8)func_801F9DF4(arg0, arg1);
  }
  return 0;
}

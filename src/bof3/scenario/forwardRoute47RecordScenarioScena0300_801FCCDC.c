#include "bof3/bof3.h"

extern u16 D_80143F00;

s32 func_801FCD1C(s32 arg0, s32 arg1);

/* @source 0x801FCCDC
 * @behavior Route-0x47 gate in front of the overlay's packed-record handler
 * func_801FCD1C: when the shared route word D_80143F00 reads 0x47 (71) it
 * forwards its own two incoming packed record words unchanged (no argument
 * register is set up anywhere in its body, and the callee consumes both: it
 * matches the first word's high half against the record kinds 0x0E..0x11 and
 * the second against 0x490000, then runs func_8015C088 and requests the shared
 * primary state 8 with sub-state 2, D_80146874 = 8 / D_80146875 = 2 /
 * D_80146876 = 0x1E) and returns the sign-extended low byte of that result, so
 * it reports 1 only on that path; for every other route value it reports 0,
 * materialised as `addu $v0,$zero,$zero` in the jump delay slot of the return-0
 * path, without reading or writing any other state. The 0x18-byte frame only
 * keeps $ra across the call, which is sunk into the branch delay slot, and the
 * 64 bytes are the same instruction shape as the unlifted SCENA04 gate at
 * 0x801F9DB4, whose route constant is 0x28 and whose callee is func_801F9DF4.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 forwardRoute47RecordScenarioScena0300_801FCCDC(s32 arg0, s32 arg1) {
  if (D_80143F00 == 0x47) {
    return (s8)func_801FCD1C(arg0, arg1);
  }
  return 0;
}

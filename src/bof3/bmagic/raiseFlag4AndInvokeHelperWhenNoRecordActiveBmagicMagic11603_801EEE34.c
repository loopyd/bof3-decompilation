#include "bof3/bof3.h"

/* Record sweep of this overlay (0x801EF118): walks the 0x80 records of the
 * 0x3C-byte-stride pool at 0x801EF2A8, draws every record whose lead byte at
 * offset 0 is non-zero through func_801EEE7C and returns non-zero once it has
 * handled at least one record, or zero when every lead byte reads 0. Still an
 * asm span of this target, so this lift only needs the call-site prototype. */
s32 func_801EF118(void);

void func_801E5988(void);

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind
 * the same word (BATTLE_GLOBAL_HALF_62E8); it lives outside this overlay, so it
 * keeps its address-qualified name. The original computes the address into one
 * register and keeps it across the read-modify-write, so it is reached through
 * a local pointer. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEE34
 * @behavior Idle step of this overlay, the fourth word (index 3) of the
 *           four-entry code pointer run at 0x801EEC04 in this target's leading
 *           table segment, whose words are 0x801EEC80, 0x801EED2C, 0x801EEDB0
 *           and 0x801EEE34; no dispatched arm or jal inside this payload
 *           reaches it. It runs the record sweep at 0x801EF118, which walks the
 *           0x80 records of the 0x3C-byte-stride pool at 0x801EF2A8 and returns
 *           non-zero once it has drawn at least one record whose lead byte at
 *           offset 0 reads non-zero. Only when that result masked to its low
 *           byte is zero - no record was active - does it raise bit 0x4 of the
 *           shared battle global halfword at 0x801462E8 and then invoke the
 *           shared helper at 0x801E5988 with no arguments. The 0x18-byte frame
 *           only keeps $ra across the two calls, the second call is reached
 *           only on the zero path, and nothing is returned. The masked-result
 *           shape is the one already exact in
 *           resetWorkStateWhenNoRecordActiveScenarioScena0900_801F73F0
 *           (0x801F73F0).
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4AndInvokeHelperWhenNoRecordActiveBmagicMagic11603_801EEE34(void) {
  if ((func_801EF118() & 0xFF) == 0) {
    u16 *flags = (u16 *)&D_801462E8;

    *flags |= 4;
    func_801E5988();
  }
}

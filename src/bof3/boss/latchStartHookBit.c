#include "bof3/bof3.h"

/* Shared battle flag byte of the main executable: the original loads it with
 * `lbu` and tests bit 0x2. */
extern u8 D_80146328; /* @source 0x80146328 @kind unknown */

/* Shared byte of the main executable at 0x8014864C, which this hook latches to
 * the literal 2 and the battle clear routine 0x801D67EC resets to zero. */
extern u8 D_8014864C; /* @source 0x8014864C @kind data */

/* Shared front-end action helper of the main executable, outside this payload:
 * it starts the action its argument ids. */
void func_80150224(s32 action); /* @source 0x80150224 */

/* @source 0x800C1D88
 * @behavior Third battle hook of this overlay, the entry the same overlay's
 * installer 0x800C1D50 publishes into the shared battle hook slot 0x801463AC,
 * whose sibling hook 0x800C2B98 of BIN/BOSS/BOSS025.EMI#13 documents that slot's
 * argument as the hook mode byte. The original masks the argument to its low
 * byte and, only when that mode is zero and bit 0x2 of the shared battle flag
 * byte 0x80146328 is set, latches the shared byte 0x8014864C to 2 and starts
 * front-end action 0x19 through the shared action helper func_80150224; every
 * path returns the zero result the hook slot expects. The 0x4C bytes use a
 * 0x18-byte frame that keeps only $ra across the one call, and both `lbu` and
 * the second branch keep a `nop` in their delay slots.
 *
 * No word of this payload names 0x800C1D88: neither a `jal` immediate nor a
 * pointer word reaches it, so the installer's slot store is its only reference
 * and the body takes its argument role from the slot, not from a call site.
 * No byte-identical copy of these 0x4C bytes exists in any of the extracted
 * payloads under out/binaries, this span's own offset being the single hit, so
 * the clone lever is declined on measurement and the body was written from
 * this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
u32 latchStartHookBit(
    u8 mode) {
  if (mode == 0) {
    if ((D_80146328 & 0x2) != 0) {
      D_8014864C = 2;
      func_80150224(0x19);
    }
  }
  return 0;
}

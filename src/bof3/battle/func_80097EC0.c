#include "bof3/battle/battle15_internal.h"

/* @behavior Initialises the primary battle selection-grid scratch band at
 * 0x80148570 from the active message-slot work record and the saved
 * group/page/cursor row of the 0x801454F4 table: the head byte, +0x04 and
 * +0x06 are written through the 0x80148570 base, the saved row through its own
 * symbols, and 0x80148584 receives bit 1 of the active selection slot flag word
 * when bits 1 and 17 of that word are both set.
 * @source 0x80097EC0
 * @status partial
 * @match 67.11
 * @residual non-exact live audit: 51/74 instructions; 296 original bytes versus
 *   304 current. Symbol-relative accesses now reproduce the original's
 *   %hi/%lo form for the base and the 0x801454F4 row. The remaining difference
 *   is allocation: the original keeps the 0x80148570 base in $a3 (this build
 *   uses a scratch temp) and has no frame, while this build still reserves an
 *   unused 16-byte frame after the flag test.
 *   Measured shapes: precomputing the bit as u32 before the test (45/74, 308),
 *   the same after the early flag read (49/74, 304), an s16 bit whose
 *   conversion sits outside the arm (49/74, 304 early, 45/74, 308 late), and
 *   storing the bit unconditionally before a conditional zero (35/74, 292 and
 *   32/74, 288). Taking the conversion out of the arm left the frame and the
 *   two-arm structure intact, so the dead frame follows register pressure and
 *   not the store's conversion point; only the unconditional-store family
 *   drops it, and that costs the branch pair the original has. Residual is
 *   allocator-owned; no clean-C lever left in lane scope.
 */
void func_80097EC0(void) {
  BattleSelectionState *state;
  u8  *active;
  u8   slot;
  u32  flags;

  state = &D_80148570;
  state->first = 1;
  D_80148571 = 8;
  D_80148572 = 2;
  D_80148573 = 2;
  slot = D_801EBF08[5];
  active = D_801EB4D8;
  flags = *(u32 *)(active + 0x10);
  D_8014857A = slot;
  D_8014857B = D_801454F4[slot * 3];
  D_80148580 = D_801454F4[slot * 3 + 1];
  D_8014857D = 0xFF;
  D_80148578 = 2;
  D_80148579 = 0;
  D_8014857C = D_801454F4[slot * 3 + 2];

  if ((flags & 0x20002u) != 2u) {
    D_80148584 = 0;
  } else {
    D_80148584 = flags & 2u;
  }

  state->unk_04 = 0x140;
  state->unk_06 = 0x3F;
  D_801485DC = 0;
  D_801485DD = 8;
  D_801485DE = 0;
  D_80148656 = D_801EBF08[5];
}

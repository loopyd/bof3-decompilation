#include "bof3/battle/battle15_internal.h"

/* @source 0x8009DE8C
 * @behavior Negates the halfword selected by the active battler index byte and
 *   stores the result in the field at +0x04 of the selection record pointer
 *   D_801463A0: below index 3 the halfword comes from the 0x140-stride
 *   local-work record field above 0x80145F18, at index 3 and above from the
 *   0x118-stride enemy-work record halfword above 0x801EB6C4. When the
 *   enemy-work halfword is the 0xFFFF sentinel the stored field becomes -1 and
 *   the matching enemy status byte at 0x801EB72C gains bit 2.
 * @status partial
 * @match 94.74
 * @residual 54/57 instructions, 228/228 bytes: 3 instructions differ only in
 *   delay-slot placement. The else path's negation fills the pointer-load
 *   delay slot after `lw v1` and the arm jumps straight to the store, while the
 *   original lifts it into the `bne` delay slot and keeps a `nop` in the
 *   load-delay slot (then-arm `j` target 0x8009DF64 vs 0x8009DF60). Clean-C
 *   rungs tried and rejected: direct volatile reads with u8/u32 locals (extra
 *   `andi`, folded address), non-volatile view without an index hoist (loads
 *   CSEd), index/negation in separate statements (cross-jump tail lost),
 *   inverted sentinel test and inverted arm order (layout regressed). Profile
 *   (`bin/flag-search`) and permuter rungs are opt-in and were not authorized
 *   for this mission.
 */
void func_8009DE8C(void) {
  u8* state;
  u16 value;

  /* Non-volatile view: the original reads the byte once for the compare and
   * the index (the sentinel path re-reads it, which the volatile cast below
   * forces). A volatile read here widens through `andi` and drops the shared
   * address register. */
  state = (u8*)&D_80146394;
  if (*state < 3) {
    ((s16*)D_801463A0)[2] =
        -*(u16*)(D_80145F18 + *state * 0x140 + 8);
  } else {
    u32 enemy_index;

    /* Kept as an arithmetic step: an inlined `(*state - 3) * 0x118` is
     * distributed into the 0x801EB6C4 %lo addend instead of emitting
     * `addiu v1,v1,-3`. */
    enemy_index = *state - 3;
    value = *(u16*)(D_801EB6C4 + enemy_index * 0x118 + 0xC);
    if (value == 0xFFFF) {
      ((s16*)D_801463A0)[2] = -1;
      D_801EB72C[*(volatile u8*)state - 3].status |= 4;
      return;
    }
    ((s16*)D_801463A0)[2] = -value;
  }
}

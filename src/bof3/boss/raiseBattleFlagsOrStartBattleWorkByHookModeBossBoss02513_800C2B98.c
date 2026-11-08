#include "bof3/bof3.h"

/* Shared main-program cells this dispatcher touches. 0x80146328 is the battle
 * flag byte the sibling lifts of this overlay test for the 0x2 bit, and
 * 0x801462ED is the flag byte five above the shared battle flag halfword
 * 0x801462E8. 0x8014630B is the byte table the ranked-owner cursor 0x80146322
 * indexes; 0x80145FB4 is the first word of the 0x140-stride local work run
 * (0x80145E90 + 0x124) that the same kind byte selects. The two presence
 * halfwords 0x801EB6B2 and 0x801EB7CA sit one 0x118-byte record stride apart in
 * the enemy work segment, and 0x801EB748 is the battle work record this
 * dispatcher publishes through the shared scratchpad pointer cell. */
extern u8 D_801462ED;     /* @source 0x801462ED @kind unknown */
extern u8 D_80146328;     /* @source 0x80146328 @kind unknown */
extern u8 D_80146322;     /* @source 0x80146322 @kind unknown */
extern u8 D_8014630B[];   /* @source 0x8014630B @kind table */
extern u8 D_80145FB4[];   /* @source 0x80145FB4 @kind table */
extern u16 D_801EB6B2;    /* @source 0x801EB6B2 @kind unknown */
extern u16 D_801EB7CA;    /* @source 0x801EB7CA @kind unknown */
extern u8 D_801EB748[];   /* @source 0x801EB748 @kind unknown */
extern u8* g_battle_work; /* @source 0x1F800044 @kind data */

u32 func_8014D8D4(u8 arg0);

/* @source 0x800C2B98
 * @behavior Overlay hook-mode dispatcher, the third member of the handler trio
 *           the sibling installer 0x800C2B60 publishes into the shared battle
 *           hook slot 0x801463AC. Masks the hook mode byte and switches through
 *           the six-entry jump table at 0x800C1834; modes 1, 3, 4 and 5 fall
 *           straight to the one return. Mode 0 raises the 0x4 bit of the shared
 *           battle flag byte 0x80146328 and sets bit 0x1 or bit 0x2 of the flag
 *           byte 0x801462ED, choosing by which of the two 0x118-strided enemy
 *           presence halfwords (0x801EB6B2, 0x801EB7CA) holds the 0x4000 bit;
 *           when neither does, and only while 0x80146328 is still zero and bit
 *           0x8 of 0x801462ED clear, it reads the kind byte 0x8014630B at the
 *           ranked-owner cursor 0x80146322 and, for a kind below 3 whose
 *           0x140-stride work record word has bits 0x1 and 0x2 clear, raises the
 *           same 0x4 bit of both flag bytes. That arm then drops bits 0x20 and
 *           0x8 of 0x801462ED when bit 0x20 was set. Mode 2 runs only while bit
 *           0x4 of 0x80146328 is clear and bit 0x10 of 0x801462ED is set: it
 *           clears bit 0x10, publishes the battle work record 0x801EB748 through
 *           the scratchpad pointer cell 0x1F800044, submits effect 0xC through
 *           func_8014D8D4 and raises bit 0x8. Every path returns zero, which the
 *           callers ignore; no jal in the payload names this function, so the
 *           hook slot is its only caller.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Two clean-C shapes carry the match. The two presence arms load both flag
 * bytes into block-typed temporaries, OR them and only then store both, which
 * is the order the original emits (both loads first, both ORs after them) and
 * what lets the compiler merge the identical store pair of the two arms into
 * the one shared tail at 0x800C2C34 that each arm jumps or falls into; storing
 * each byte at its OR site instead gives each arm its own stores and fails at
 * the second arm. The pointer to 0x80146328 takes its initializer in the
 * declaration, because the arm-local web that form creates is the one the
 * allocator hands $v0, with the two byte values following in $v1 and $a0.
 * The switch emits the six-entry `.rodata` jump table the original links at
 * 0x800C1834 inside the leading table data, so target.toml carries that
 * address as a reviewed section placement for this function.
 */
u32 raiseBattleFlagsOrStartBattleWorkByHookModeBossBoss02513_800C2B98(u8 mode) {
  u8* battle_flags;
  u8* phase_flags;
  u8 kind;
  u32 record_flags;

  switch (mode) {
  case 0:
    if ((D_801EB6B2 & 0x4000) != 0) {
      {
        u8* battle_flags = &D_80146328;
        u8 flags_value;
        u8 phase_value;

        flags_value = *battle_flags | 0x4;
        phase_value = D_801462ED | 0x1;
        *battle_flags = flags_value;
        D_801462ED = phase_value;
      }
    } else if ((D_801EB7CA & 0x4000) != 0) {
      {
        u8* battle_flags = &D_80146328;
        u8 flags_value;
        u8 phase_value;

        flags_value = *battle_flags | 0x4;
        phase_value = D_801462ED | 0x2;
        *battle_flags = flags_value;
        D_801462ED = phase_value;
      }
    } else {
      battle_flags = &D_80146328;
      if (*battle_flags == 0 && (D_801462ED & 0x8) == 0) {
        kind = D_8014630B[D_80146322];
        if (kind < 3) {
          record_flags = *(u32*)(void*)(D_80145FB4 + kind * 0x140);
          if ((record_flags & 0x1) == 0) {
            if ((record_flags & 0x2) == 0) {
              *battle_flags |= 0x4;
              D_801462ED |= 0x4;
            }
          }
        }
      }
      phase_flags = &D_801462ED;
      if ((*phase_flags & 0x20) != 0) {
        *phase_flags &= 0xD7;
      }
    }
    break;
  case 1:
    break;
  case 2:
    if ((D_80146328 & 0x4) == 0 && (D_801462ED & 0x10) != 0) {
      D_801462ED &= 0xEF;
      g_battle_work = D_801EB748;
      func_8014D8D4(0xC);
      D_801462ED |= 0x8;
    }
    break;
  case 3:
    break;
  case 4:
    break;
  case 5:
    break;
  }

  return 0;
}

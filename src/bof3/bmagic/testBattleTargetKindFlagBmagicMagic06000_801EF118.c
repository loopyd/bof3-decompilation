#include "bof3/bof3.h"

/* 0x118-byte local panel record row: the kind byte read here comes first, the
 * rest is unnamed. The battle target declares the same row shape as its private
 * BattleLocalPanelRecord (include/bof3/battle/battle15_internal.h), whose kind
 * byte func_8009CFEC and func_801DA69C read through the same 0x118 stride, so
 * this overlay binds the shape it reads rather than including that target's
 * header. */
typedef struct BmagicMagic06000LocalPanelRow {
  u8 kind;
  u8 unk_01[0x118 - 1];
} BmagicMagic06000LocalPanelRow;

/* Local panel record array in main RAM, outside this overlay's image. Bound
 * through this target's generated undefined_syms_auto.txt, which the remaining
 * asm sibling func_801EF0B8 still references, so no target-map row is added. */
extern BmagicMagic06000LocalPanelRow D_801EB6AC[]; /* @source 0x801EB6AC @kind table */

/* Per-kind flag words in main RAM, 256 bits over eight words, indexed by the
 * record's kind byte the same way the battle lifts index the neighbouring flag
 * words 0x80144F60 and 0x80144F80 (func_800AD044, func_800AA874). */
extern u32 D_80144FA0[]; /* @source 0x80144FA0 @kind table */

/* Battle target selector byte in main RAM, outside this overlay's image. The
 * exact bmagic siblings seedWorkMotionFromBattlerRecordAndAdvanceWorkByte2BmagicMagic05403_801EF238
 * and raiseBattleTargetFlag20ClearBytes9AThenAdvanceWorkByte1BmagicMagic09403_801EEF1C
 * read the same byte; the -3 here is this overlay's record base offset. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* @source 0x801EF118
 * @behavior Tests one bit of the per-kind flag table at 0x80144FA0 for the
 *           current battle target. It reads the battle target selector byte
 *           0x80146384, takes the kind byte of that target's 0x118-byte local
 *           panel record at 0x801EB6AC indexed by (selector - 3), and returns
 *           nonzero when bit (kind & 0x1F) of the flag word
 *           D_80144FA0[kind >> 5] is set, zero when it is clear. The bit test
 *           is the same tail shape the exact battle lift func_800AA874 uses on
 *           the neighbouring flag word. The caller at 0x801EF01C masks the
 *           result with 0xFF and gates work byte 0x04 on it. Leaf with no
 *           frame; it takes no arguments and writes no state.
 * @status exact
 * @match 100.00
 * @residual none
 */
u32 testBattleTargetKindFlagBmagicMagic06000_801EF118(void) {
  u8 kind;

  kind = D_801EB6AC[D_80146384 - 3].kind;
  return (D_80144FA0[kind >> 5] & (1 << (kind & 0x1F))) != 0;
}

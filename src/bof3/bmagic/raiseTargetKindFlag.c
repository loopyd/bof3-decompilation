#include "bof3/bof3.h"

/* 0x118-byte local panel record row: this lift reaches only the kind byte at
 * +0x00, which is the bit index half of the per-kind flag pair. The two
 * bmagic siblings checkTargetKindFlag and
 * raiseTargetDeltaRoll
 * both measured exact and declare the same row shape in their own files; this
 * one binds the fields it reads rather than sharing a header. */
typedef struct BmagicMagic06000LocalPanelRow {
  u8 kind;
  u8 unk_01[0x118 - 1];
} BmagicMagic06000LocalPanelRow;

/* Local panel record array in main RAM, outside this overlay's image. Bound
 * through this target's generated undefined_syms_auto.txt, which the two
 * sibling lifts above resolve the same way, so no target-map row is added. */
extern BmagicMagic06000LocalPanelRow D_801EB6AC[]; /* @source 0x801EB6AC @kind table */

/* Per-kind flag words in main RAM, 256 bits over eight words, indexed by the
 * record's kind byte the same way the battle lifts index the neighbouring flag
 * words 0x80144F60 and 0x80144F80 (func_800AD044, func_800AA874). The sibling
 * checkTargetKindFlag, measured exact, reads the
 * same table and the same word at the same index. */
extern u32 D_80144FA0[]; /* @source 0x80144FA0 @kind table */

/* Battle target selector byte in main RAM, outside this overlay's image. The
 * -3 here is this overlay's local panel record base offset, the same reduction
 * checkTargetKindFlag takes. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* @source 0x801EF0B8
 * @behavior Sets one bit of the per-kind flag table at 0x80144FA0 for the
 *           current battle target. It reads the selector byte 0x80146384,
 *           takes the kind byte at the start of that target's 0x118-byte local
 *           panel record at 0x801EB6AC indexed by (selector - 3), forms the
 *           flag word address D_80144FA0[kind >> 5], shifts the constant 1 left
 *           by (kind & 0x1F) and stores the word back with that bit set, the
 *           store filling the return delay slot. It takes no arguments and
 *           returns nothing: the shifted constant is what the return register
 *           happens to hold, not a result any caller reads. Leaf with no frame
 *           and no branch; its one caller is the work-byte-1 handler at
 *           0x801EEF50, which reaches it with the jal at 0x801EF074 on the arm
 *           where both the kept roll and record byte 0x03 are nonzero, right
 *           after checkTargetKindFlag reported
 *           the bit clear. The pointer cell 0x1F800044 that handler stores
 *           after the call is untouched here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseTargetKindFlag(void) {
  u8 kind;

  kind = D_801EB6AC[D_80146384 - 3].kind;
  D_80144FA0[kind >> 5] |= 1u << (kind & 0x1F);
}

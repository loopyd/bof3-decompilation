#include "base/types.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work like the exact siblings of this target declare it. The body
 * reads the cell once after the three calls and once more inside the arm, each
 * with its own lui / lw pair, because a store through the loaded pointer may
 * alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle record pointer in main RAM, outside this overlay image. This
 * target's generated undefined_syms_auto.txt binds the address and the sibling
 * copyRecordWordPairAdd2000ToWord34Add100ToHalfword3EThenSetDirectionToward
 * SlotRecordFromByteAStoreWord14ClearByte9AndAdvanceWorkByte2 reads the same
 * byte 0xA: the queued action-slot index the record currently holds. The roles
 * beyond what the body does with the byte are not proven, so the address stays
 * address-anchored. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Queued action-slot store based at 0x801EC330 in main RAM, entries of the
 * 0x78-byte stride this body scales the record byte 0xA by. Unlike the field
 * accesses of its siblings, this body materialises the array base itself -
 * lui %hi, addiu %lo - and adds the scaled index afterwards, so the address is
 * reached as one base plus an index and the generated undefined_syms_auto.txt
 * row for it stays binding. */
extern u8 D_801EC330[]; /* @source 0x801EC330 @kind data */

/* Record helper of the main executable at 0x800AF2B4, not lifted, so its effect
 * stays unestablished. The original passes a record base and a constant; this
 * step passes the queued entry and 3. */
extern void func_800AF2B4(u8 *record, u32 mode);

/* Shared main-executable helper at 0x800AF764. This overlay calls it with no
 * arguments and no result as a step ends; the generated undefined_funcs_auto.txt
 * row for that address binds it. */
void func_800AF764(void);

/* This overlay's own draw step at 0x801EF6A0, still an asm row in this target:
 * it selects a draw mode from GetGraphType, installs it on the prim cursor and
 * queues the record's primitives. Kept at its raw name because the span has no
 * authored body. */
void func_801EF6A0(void);

/* @source 0x801EF57C
 * @behavior Work step handler of one action-slot sequence of this overlay. It
 *           reads the shared battle record pointer from 0x801EB4E0 and takes
 *           that record byte 0xA, the index of the queued action-slot entry it
 *           holds, then calls the main executable record helper func_800AF2B4
 *           with the entry address - the 0x78-byte-stride store based at
 *           0x801EC330 scaled by that index - and the constant 3. It follows
 *           with the shared main executable step helper func_800AF764 and this
 *           overlay draw step func_801EF6A0, both with no arguments and no
 *           result. After those three calls it reads the scratchpad work-object
 *           pointer cell 0x1F800044 once and decrements work-object byte 0xA
 *           through it with a byte-width read-modify-write (lbu / addiu -1 /
 *           sb), then tests the truncated byte it just stored (andi 0xFF of the
 *           stored register, no second load of the cell) and returns while that
 *           countdown is still nonzero. On expiry it re-reads the cell, since a
 *           store through the loaded pointer may alias it, and increments
 *           work-object byte 0x2, the state byte that advances this overlay work
 *           handler sequence. The 0x18-byte frame keeps ra at 0x10 across the
 *           three calls; it takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeRecordHelper3StepAndDrawCountdownWorkByteAAndAdvanceWorkByte2WhenZeroBmagicMagic08103_801EF57C(void) {
  u32 slot;
  u8 countdown;

  slot = D_801EB4E0[0xA];
  func_800AF2B4(&D_801EC330[slot * 0x78], 3u);
  func_800AF764();
  func_801EF6A0();
  countdown = g_battle_work[0xA] - 1;
  g_battle_work[0xA] = countdown;
  if (countdown == 0) {
    g_battle_work[2]++;
  }
}

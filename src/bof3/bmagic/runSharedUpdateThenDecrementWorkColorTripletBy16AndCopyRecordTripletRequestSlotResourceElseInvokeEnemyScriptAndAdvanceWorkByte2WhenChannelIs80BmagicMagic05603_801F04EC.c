#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * handlers, bound at 0x1F800044 by this target's map. It keeps the volatile
 * pointer spelling the exact bmagic siblings
 * runSharedUpdateThenDecrementWorkByte9AndRaiseFlag20SetByte5CToOneClearWork
 * ColorTripletAndAdvanceWorkByte2BmagicMagic05603_801F042C and
 * decrementWorkColorTripletBy16ThenCopyRecordTripletAndAdvanceWorkByte2When
 * ChannelIs80BmagicMagic05603_801F00C8 use: every statement that goes through it
 * re-reads the cell, because a store through the pointer it supplied may alias
 * the cell itself, and the named symbol lets the assembler fold the constant
 * address into each load (lui $vN,0x1F80 / lw $vN,68($vN)). This body also
 * publishes the selected party record into the cell for the duration of one
 * helper call and writes the saved work object back afterwards. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Acting battler byte of the main executable, outside this overlay's image.
 * Party slots are 0, 1 and 2 and the enemy panel slots are 3 and above, so the
 * unsigned compare below against 3 selects the party work base this span
 * publishes. The original reads it once, after the record-triplet copy and
 * before the compare, so the declaration is volatile to keep that read at the
 * position its bytes show instead of letting it fill the copy's load-delay nop.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is needed. */
extern volatile u8 D_80146374; /* @source 0x80146374 @kind unknown */

/* Shared battle record pointer cell in main RAM, outside this overlay's image.
 * The world and bmagic lifts read the same cell for the word triplet at 0x34,
 * 0x38 and 0x3C. The original re-reads the cell with a fresh lui and lw pair
 * before each of the three stores rather than keeping the pointer in a register,
 * so the declaration keeps the volatile pointer spelling the work-object cell
 * uses. Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is needed. */
extern u8 * volatile D_8014637C; /* @source 0x8014637C @kind unknown */

/* Base of the three 0x140-byte-stride party work records the acting battler byte
 * selects. The plchar lifts index the same base as D_80145E90[i * 0x140] and the
 * exact bmagic magic151 lift
 * publishWorkRecordSelectSequenceByte8Plus3CThenAdvanceWorkByte3BmagicMagic15103_801EEEA8
 * indexes it the same way. Splat binds the address through this target's
 * generated undefined_syms_auto.txt, so no target-map row is needed. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind unknown */

/* Shared no-argument update helper of the main executable; the bmagic and plchar
 * lifts declare the same prototype and this caller keeps none of its answer. */
extern void func_8014D978(void); /* @source 0x8014D978 */

/* Shared resource-request helper of the main executable. The plchar lifts pass
 * the published work object's byte 8; this span passes that byte widened by
 * 0x1C and masked to eight bits, which is why the declaration narrows its
 * argument to byte width. Splat binds the address through this target's
 * generated undefined_funcs_auto.txt, so no target-map row is needed. */
extern void func_8014D6B8(u8 resource);

/* Shared battle helper of the main executable: it publishes the 0x118-byte enemy
 * work record at 0x801EB2E8 indexed by its first argument as the current enemy
 * work and runs the enemy script byte helper for the second argument. Splat
 * binds the address through this target's generated undefined_funcs_auto.txt. */
extern void func_801E257C(u8 index, u8 scriptByte);

/* @source 0x801F04EC
 * @behavior Step 6 of the nine-word code pointer run based at 0x801EEC10, which
 *           the per-frame dispatcher copies into its frame and selects from with
 *           work byte 2, so no jal in this payload names this address. The body
 *           first runs the shared no-argument update helper func_8014D978 and
 *           discards its answer, then fades the scratchpad work object: it
 *           re-reads the work-object pointer cell at 0x1F800044 before each of
 *           the three colour channels and subtracts 0x10 from the bytes at 0x5D,
 *           0x5E and 0x5F at byte width with no floor test before those stores.
 *           It then re-reads the cell into a saved register and tests the signed
 *           channel at 0x5D; only when that channel reads exactly -0x80 does the
 *           rest of the body run. That arm copies words 0x34, 0x38 and 0x3C of
 *           the shared battle record read through 0x8014637C into the work
 *           object, re-reading that record pointer before each of the three
 *           stores. It then tests the acting battler byte at 0x80146374 with an
 *           unsigned compare against 3: for a party slot it forms the record at
 *           0x80145E90 indexed by that byte at the 0x140-byte stride, publishes
 *           that record as the scratchpad work object, calls the shared resource
 *           request func_8014D6B8 with the record byte 8 plus 0x1C masked to
 *           eight bits, restores the previously saved work object into the cell
 *           and calls func_8014D6B8 again with that object's byte 8 plus 0x1C.
 *           For an enemy slot it instead calls the shared battle helper
 *           func_801E257C with the acting battler byte as enemy work index and 4
 *           as the script byte. Both arms then re-read the cell and increment
 *           work byte 2 at byte width, advancing the work-byte-2 dispatcher to
 *           step 7. The 0x18-byte frame keeps the return address at 0x14 and the
 *           saved work object at 0x10. Takes no arguments, returns nothing and
 *           reads or writes no other state.
 * @status partial
 * @match 93.48
 * @residual One instruction over the original: the acting-battler read at
 *           0x80146374 loads into a scratch register and is copied to the
 *           argument register with an extra andi 0xFF, where the original lbu
 *           lands in that register directly. The three record-triplet reads and
 *           stores, the 0x140-stride party record form, both helper calls, the
 *           restore of the saved work object and the byte-2 advance all match
 *           instruction for instruction. The 0x16C-byte span is not byte
 *           identical to any span of the 396 payloads under out/binaries, so it
 *           was authored from the original bytes: the same target's 0x801F00C8
 *           supplied the fade, record-triplet copy and byte-2 advance, and
 *           0x801F042C the leading shared-update call and frame.
 */
void runSharedUpdateThenDecrementWorkColorTripletBy16AndCopyRecordTripletRequestSlotResourceElseInvokeEnemyScriptAndAdvanceWorkByte2WhenChannelIs80BmagicMagic05603_801F04EC(void) {
  u8 *work;
  u8 *record;
  volatile s32 *target;
  u8 index;

  func_8014D978();
  g_battle_work[0x5D] -= 0x10;
  g_battle_work[0x5E] -= 0x10;
  g_battle_work[0x5F] -= 0x10;

  work = g_battle_work;
  if (((s8 *)work)[0x5D] == -0x80) {
    target = (volatile s32 *)work;
    target[13] = ((s32 *)D_8014637C)[13];
    target[14] = ((s32 *)D_8014637C)[14];
    target[15] = ((s32 *)D_8014637C)[15];

    index = D_80146374;
    if (index < 3u) {
      record = &D_80145E90[index * 0x140];
      g_battle_work = record;
      func_8014D6B8(record[8] + 0x1C);
      g_battle_work = work;
      func_8014D6B8(work[8] + 0x1C);
    } else {
      func_801E257C(index, 4);
    }

    g_battle_work[2]++;
  }
}

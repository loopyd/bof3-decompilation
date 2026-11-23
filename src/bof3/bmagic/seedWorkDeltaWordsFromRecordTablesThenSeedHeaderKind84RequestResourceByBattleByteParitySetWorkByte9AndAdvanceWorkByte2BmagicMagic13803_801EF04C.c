#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work. Every store group in the original re-materialises the cell
 * and loads it again (fourteen lui/lw pairs for the seeding block), because a
 * byte store through the loaded pointer may alias the cell itself, so the
 * accesses go through this name and each group keeps its own load. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Acting battler index byte in main RAM, outside this overlay's image. The
 * original materialises its address once into $a1 and re-reads the byte with a
 * fresh lbu after every store group, because the stores that sit between the
 * reads go through the work pointer, which may alias this byte. Its base is
 * kept in a register across the whole body, which is why it is addressed as a
 * one-element row rather than as a scalar. */
extern u8 D_80146374[]; /* @source 0x80146374 @kind unknown */

/* Shared battle record pointer cell in main RAM, outside this overlay's image.
 * The original loads the pointer it holds and reads its byte 0x9; this target's
 * generated undefined-syms map already binds the address because the overlay's
 * other spans load it too, so the name needs no target-map row. The exact
 * sibling publishBattleWorkRecordPointerInvokeUpdateHelperThenForwardBattleByte
 * RaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic13803_801EEED8 declares
 * the same cell for the same role. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Party record block of the shared main executable, addressed with a
 * 0x140-byte stride. This body reads the halfwords at stride offsets 0x2E and
 * 0x30 and materialises each base separately (its own lui %hi / addu / load %lo
 * triple), so each is declared and addressed at its own address. The field
 * roles at those offsets are not established, so they stay offset-anchored. */
extern u8 D_80145EBE[]; /* @source 0x80145EBE @kind unknown */
extern u8 D_80145EC0[]; /* @source 0x80145EC0 @kind unknown */

/* Record block of the shared main executable that the branch for an acting
 * battler index of 3 or more indexes with a 0x118-byte stride, read at the same
 * two 0x2E and 0x30 offsets. Its roles are not established either, so the two
 * accessed words keep their own addresses. */
extern u8 D_801EB65E[]; /* @source 0x801EB65E @kind unknown */
extern u8 D_801EB660[]; /* @source 0x801EB660 @kind unknown */

/* Shared battle byte at 0x80146384, read unsigned once for the requested
 * resource selector. It lives outside this overlay, so it keeps its
 * address-qualified name. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Resource request wrapper of the shared main executable, outside this
 * payload. The sibling lift seedWorkHeaderKind84SelectorB4SelectCueByByteAParity
 * RequestResourceAdvanceWorkByte2BmagicMagic13803_801EF5D0 calls the same
 * address, whose generated undefined_funcs_auto.txt row binds it here. */
extern void func_8014D6B8(u32 selector); /* @source 0x8014D6B8 @kind function */

/* @source 0x801EF04C
 * @behavior Overlay header handler, entry index 3 of the nine-word handler
 *           pointer run at 0x801EEC04 (payload word 0x801EEC10), so the
 *           dispatcher at 0x801EEFA4 invokes it with no arguments when unsigned
 *           work byte 2 is zero. It selects one of two indexed record blocks
 *           with the shared acting-battler byte at 0x80146374: while that byte
 *           is below 3 the record base is the party block whose halfword 0x2E
 *           sits at 0x80145EBE and halfword 0x30 at 0x80145EC0 with a
 *           0x140-byte stride, and while it is 3 or more the base is the block
 *           whose same halfwords sit at 0x801EB65E and 0x801EB660 with a
 *           0x118-byte stride indexed by the byte minus 3. Bit 0 of byte 9 of
 *           the shared battle record at 0x801EB4E0 selects the amount 0x28
 *           against 0x14; the chosen amount is subtracted from the work
 *           object's own halfword 0x2E first, that result is subtracted from
 *           the selected record halfword 0x2E, and the difference is scaled by
 *           16 and divided by 8 into work word 0xC. A second read of the
 *           acting-battler byte then selects the record halfword 0x30, whose
 *           difference against the work object's own halfword 0x30 is scaled
 *           the same way into work word 0x10. The seeding block that follows
 *           re-reads the work pointer for each store group: it stores 0x1E into
 *           work byte 0x25, moves work halfwords 0x2E and 0x30 left by four
 *           into work words 0x18 and 0x1C, clears work bytes 0x26, 0x28, 0x2A,
 *           0x2B, 0x5C, 0x5D, 0x5E and 0x5F together with work halfword 0x2C, and
 *           stores 1 into work byte 0x48, 0xA0 into byte 0x27, 0x84 into byte
 *           0x24 and 3 into byte 0x29. It then requests resource
 *           ((0x80146384 - 3) & 1) through the shared wrapper func_8014D6B8,
 *           stores 3 into work byte 9 and advances unsigned work byte 2 by one,
 *           so the next dispatch of the dispatcher above reaches its entry 1.
 *           It takes no arguments, returns nothing, and the 0x18-byte frame
 *           keeps $ra at 0x14 plus $s0 at 0x10, which holds the constant 3 that
 *           is live across the call. The roles of work words 0xC and 0x10, of
 *           work bytes 0x18/0x1C/0x24/0x25/0x27/0x29/0x48/0x5C..0x5F and of the
 *           requested resource are not established in this payload, so they
 *           stay offset-anchored and the name describes the writes.
 *
 *           No byte-identical sibling exists: a search of all 795 payloads
 *           under out/binaries for this 0x354-byte span finds only this
 *           payload's own copy at offset 0x44C, and a repeat of that scan with
 *           every j and jal target word masked finds no other copy, and so does
 *           a scan that compares opcode and register fields alone, so the clone
 *           lever is declined and no sibling supplies these statements. The
 *           statement order, the one-load-per-store-group form, the
 *           address-kept-in-a-register access of the acting-battler byte and the
 *           `* 16 / 8` scaled difference are this span's own bytes. Its
 *           neighbour seedWorkHeaderKind84SelectorB4SelectCueByByteAParity
 *           RequestResourceAdvanceWorkByte2BmagicMagic13803_801EF5D0 supplies
 *           the declaration style and the header-seeding idiom, and its own
 *           body stores the same work-header bytes in a different order and with
 *           different literals.
 *           Measured state: the 0x354-byte original is reproduced except for
 *           the duplication of the first arm of the acting-battler-below-3
 *           branch. The late jump pass cross-jumps that arm's identical tail
 *           into the second arm's copy, so the candidate emits 0x314 bytes
 *           against 0x354 and matches 168 of 213 instructions. Everything else,
 *           including the shared tail of the second branch with its own
 *           single copy, is instruction identical.
 * @status partial
 * @match 78.87
 * @residual The late jump pass merges the two arms of the acting-battler-below-3
 *           branch into one tail copy, 64 bytes and 16 instructions below the
 *           original, which keeps both copies. Statement order, per-arm locals,
 *           a coalescing result copy, an inline second index, a two-case switch
 *           and swapped load order in one arm were all measured and none
 *           inhibits the merge; the merged tail is otherwise byte identical.
 *
 *           Representation notes from the measured probes: writing the scaled
 *           difference as `* 16 / 8` in one expression folds to a single left
 *           shift by one (143 of 213, 0x2E4 bytes), and taking the subtracted
 *           amount inline makes the compiler fold the sum as table + K - halfword
 *           instead of the original's halfword - K (165 of 213, 0x2E4 bytes).
 *           Splitting the load of the record-word halfword minus the amount into
 *           its own local and leaving the scaled difference inline as a shift of
 *           four then a division by eight reaches the original's in-place
 *           rounding sequence and scores 168 of 213 at 0x314 bytes, the retained
 *           form.
 */
void seedWorkDeltaWordsFromRecordTablesThenSeedHeaderKind84RequestResourceByBattleByteParitySetWorkByte9AndAdvanceWorkByte2BmagicMagic13803_801EF04C(
    void) {
  u8 *work;
  u8 *pair;
  s32 index;
  s32 target;
  s32 off;

  if (D_80146374[0] < 3) {
    if ((D_801EB4E0[9] & 1) != 0) {
      index = D_80146374[0] * 0x140;
      work = g_battle_work;
      off = *(s16 *)(work + 0x2E) - 0x28;
      *(s32 *)(work + 0xC) = ((*(s16 *)(D_80145EBE + index) - off) << 4) / 8;
      index = D_80146374[0] * 0x140;
      target = *(s16 *)(D_80145EC0 + index);
    } else {
      index = D_80146374[0] * 0x140;
      work = g_battle_work;
      off = *(s16 *)(work + 0x2E) - 0x14;
      *(s32 *)(work + 0xC) = ((*(s16 *)(D_80145EBE + index) - off) << 4) / 8;
      index = D_80146374[0] * 0x140;
      target = *(s16 *)(D_80145EC0 + index);
    }
  } else {
    if ((D_801EB4E0[9] & 1) != 0) {
      index = (D_80146374[0] - 3) * 0x118;
      work = g_battle_work;
      off = *(s16 *)(work + 0x2E) - 0x28;
      *(s32 *)(work + 0xC) = ((*(s16 *)(D_801EB65E + index) - off) << 4) / 8;
      index = (D_80146374[0] - 3) * 0x118;
      target = *(s16 *)(D_801EB660 + index);
    } else {
      index = (D_80146374[0] - 3) * 0x118;
      work = g_battle_work;
      off = *(s16 *)(work + 0x2E) - 0x14;
      *(s32 *)(work + 0xC) = ((*(s16 *)(D_801EB65E + index) - off) << 4) / 8;
      index = (D_80146374[0] - 3) * 0x118;
      target = *(s16 *)(D_801EB660 + index);
    }
  }
  off = *(s16 *)(work + 0x30);
  *(s32 *)(work + 0x10) = ((target - off) << 4) / 8;

  work = g_battle_work;
  work[0x25] = 0x1E;
  *(s32 *)(work + 0x18) = *(s16 *)(work + 0x2E) << 4;
  *(s32 *)(work + 0x1C) = *(s16 *)(work + 0x30) << 4;
  g_battle_work[0x26] = 0;
  g_battle_work[0x48] = 1;
  g_battle_work[0x27] = 0xA0;
  g_battle_work[0x28] = 0;
  g_battle_work[0x24] = 0x84;
  g_battle_work[0x5D] = 0;
  g_battle_work[0x5E] = 0;
  g_battle_work[0x5F] = 0;
  g_battle_work[0x5C] = 0;
  g_battle_work[0x2A] = 0;
  g_battle_work[0x29] = 3;
  pair = g_battle_work;
  *(u16 *)(pair + 0x2C) = 0;
  pair[0x2B] = 0;
  func_8014D6B8((D_80146384 - 3) & 1);
  g_battle_work[9] = 3;
  g_battle_work[2]++;
}

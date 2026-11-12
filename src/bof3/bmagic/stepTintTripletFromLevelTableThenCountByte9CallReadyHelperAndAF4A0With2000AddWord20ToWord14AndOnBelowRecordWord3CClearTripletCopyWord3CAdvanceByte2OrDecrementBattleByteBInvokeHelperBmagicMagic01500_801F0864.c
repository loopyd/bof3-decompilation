#include "bof3/bof3.h"

/* Battle record pointer in main RAM, outside this overlay's image, held in the
 * word at 0x8014637C. The overlay's assembly loads that word, so the generated
 * undefined-syms map binds the address and no target-map row is needed. The
 * same-target lift callReadyHelperAndAF4A0With2000AddWord20ToWord14Accumulate
 * Halfword3ECountByte9BySignOnNegativeWord14BelowRecordWord3CRaiseFlag20Set
 * Byte5CToOneClearTripletCopyWord3CAdvanceWorkByte2BmagicMagic01500_801F0F2C
 * measured this cell exact as the record argument of helper 0x800AF4A0. */
extern u8 *D_8014637C; /* @source 0x8014637C @kind unknown */

/* Shared battle record pointer in main RAM, outside this overlay's image. The
 * overlay's assembly loads the address, so the generated undefined-syms map
 * binds it and the name needs no map row. The lifts of this target read word
 * 0x3C of that record through this cell. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through the generated undefined-syms map, so no map row is
 * needed. The same-target lift addWorkColorTripletBy16ThenDecrementGlobalWork
 * ByteBAndAdvanceWorkByte2BmagicMagic01500_801F1180 read and decremented byte
 * 0xB of the same record. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, which is how the original re-materializes
 * the cell before each access group that follows a store through the object. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Tint-step table in the overlay payload: rows of two bytes starting at
 * 0x801F1640, selected by the work-object level byte 0xB minus one, so row n
 * occupies the bytes at 0x801F1640 + n * 2 and 0x801F1641 + n * 2, read as a
 * level lookup. The table base sits at word 4 of the 0x14-byte textbin run
 * D_801F1630 that follows the code, whose generated undefined-syms row already
 * names it, so the run stays one row and no target-map row is added. The first
 * two rows read 4/3 and 3/2. */
extern u8 D_801F1640[][2]; /* @source 0x801F1640 @kind data */

/* Shared readiness helper of the main executable, outside this overlay's
 * image. Splat binds the address through the generated undefined-funcs map, so
 * no map row is needed. The sibling lifts of this target call it for its side
 * effect and discard the byte it returns, as this body does. */
u8 func_8014DAEC(void);

/* Shared main-executable helper, outside every payload. Its role is not
 * established, so the raw address name stays and no map row is needed. This
 * lift passes the record pointer read from 0x8014637C, a 0x2000 value and a
 * zero third argument, the same three argument roles the sibling 0x801F0F2C
 * passes with its own literal. */
extern void func_800AF4A0(u8 *record, s32 value, s32 extra); /* @source 0x800AF4A0 */

/* Shared main-executable helper, outside every payload. Splat binds the
 * address through this target's generated undefined-funcs map, so no map row
 * and no target-local name substitution are needed. Its role is not
 * established, so the raw address name stays. The same-target lift
 * invokeHelperBmagicMagic01500_801F1230 calls it as a thin wrapper. */
void func_801E5988(void);

/* @source 0x801F0864
 * @behavior Per-frame step of the overlay work object, the word the tabledata
 *           segment stores at 0x801EEC78: entry 29 of the 39-word code-pointer
 *           run at 0x801EEC04, between the entries at 0x801EEC74 and
 *           0x801EEC7C. It first reads the scratchpad work object through the
 *           pointer cell 0x1F800044 once and tests work byte 0xB, the level
 *           byte. When that byte is nonzero and the signed tint channel at 0x5D
 *           has not reached its -0x80 floor, it subtracts table entry
 *           (level - 1) from each colour channel: the row's first byte for
 *           channel 0x5D and its second byte for channels 0x5E and 0x5F, the
 *           guard and the first store sharing the first cell read and each
 *           later store taking its own re-read. When the level byte is zero instead, a nonzero signed
 *           channel 0x5D takes a flat drift: channel 0x5D drops by 3 and
 *           channels 0x5E and 0x5F each rise by 4. It then re-reads the cell
 *           and, unless work byte 9 already reads 0x10, decrements that
 *           countdown. It calls the shared readiness helper 0x8014DAEC and
 *           discards the byte it returns, then passes the battle record pointer
 *           read from 0x8014637C with 0x2000 and a zero third argument to
 *           helper 0x800AF4A0, then re-reads the cell and adds work word 0x20 to
 *           work word 0x14, accumulating the updated word 0x14 into work
 *           halfword 0x3E. Finally, when work word 0x14 reads negative and work
 *           word 0x3C is below word 0x3C of the record read through 0x8014638C,
 *           it ends the level: a nonzero work byte 0xB decrements byte 0xB of
 *           the shared battle work record read through 0x801EB4E0 and invokes
 *           helper 0x801E5988, while a zero byte 0xB clears the channel bytes
 *           0x5D, 0x5E and 0x5F, copies the record word 0x3C and advances work
 *           byte 2, the handler index. The cell is re-read before each store
 *           group because a store through the loaded pointer may alias the
 *           cell; the last group keeps one cell read for the word copy and the
 *           byte 2 advance. The 0x28-byte frame only keeps the return address at
 *           0x20 across the two calls, so no pointer stays live across them and
 *           the cell is loaded again afterwards. The leading guard and first
 *           store shape was copied from the exact sibling pair 0x801F0A94 and
 *           0x801F1088, and the work word 0x14, halfword 0x3E and word 0x3C
 *           sequence from the exact sibling 0x801F0F2C; this body writes its
 *           own table lookups, literals and callee set. The 0x230-byte span ends
 *           at the next subsegment, 0x801F0A94, so the row claims no trailing
 *           bytes. No jal word in the payload targets 0x801F0864 and the only
 *           word naming the address is the table entry at 0x801EEC78, so the
 *           caller that selects it lies outside the overlay. The 0x230 bytes are
 *           not byte-identical to any span of the 396 payloads under
 *           out/binaries, which one payload scan compared. The roles of work
 *           byte 0xB, work byte 9, work word 0x14, work word 0x20, work word
 *           0x3C, work halfword 0x3E, the channel bytes 0x5D to 0x5F and work
 *           byte 2 are not established, so they stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepTintTripletFromLevelTableThenCountByte9CallReadyHelperAndAF4A0With2000AddWord20ToWord14AndOnBelowRecordWord3CClearTripletCopyWord3CAdvanceByte2OrDecrementBattleByteBInvokeHelperBmagicMagic01500_801F0864(void) {
  u8 *work;
  s32 *source;
  s32 *target;

  if (D_1F800044[0xB] != 0) {
    if (((s8 *)D_1F800044)[0x5D] != -0x80) {
      ((s8 *)D_1F800044)[0x5D] -= D_801F1640[D_1F800044[0xB] - 1][0];
      D_1F800044[0x5E] -= D_801F1640[D_1F800044[0xB] - 1][1];
      D_1F800044[0x5F] -= D_801F1640[D_1F800044[0xB] - 1][1];
    }
  } else if (((s8 *)D_1F800044)[0x5D] != 0) {
    D_1F800044[0x5D] -= 3;
    D_1F800044[0x5E] += 4;
    D_1F800044[0x5F] += 4;
  }

  work = D_1F800044;
  if (work[9] != 0x10) {
    work[9]--;
  }

  func_8014DAEC();
  func_800AF4A0(D_8014637C, 0x2000, 0);

  work = D_1F800044;
  *(s32 *)(work + 0x14) += *(s32 *)(work + 0x20);
  *(u16 *)(work + 0x3E) += *(s32 *)(work + 0x14);
  if (*(s32 *)(work + 0x14) < 0) {
    if (*(s32 *)(work + 0x3C) < *(s32 *)(D_8014638C + 0x3C)) {
      if (work[0xB] != 0) {
        D_801EB4E0[0xB]--;
        func_801E5988();
      } else {
        work[0x5D] = 0;
        D_1F800044[0x5E] = 0;
        D_1F800044[0x5F] = 0;
        source = (s32 *)D_8014638C;
        target = (s32 *)D_1F800044;
        target[15] = source[15];
        ((u8 *)target)[2]++;
      }
    }
  }
}

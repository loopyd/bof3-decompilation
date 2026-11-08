#include "bof3/bof3.h"

extern int rand(void);

/* Work-record pointer cell. It is the last word of the overlay data run at
 * 0x801EF9C0 and the word just past the 96-record pool at 0x801EF9F8. This
 * function re-materialises the cell for every record access - one read before
 * the delay countdown and eleven reads inside the seed path - so the cell is
 * declared volatile, matching the exact bmagic siblings
 * dispatchWorkByte2HandlerThenDrawAndPopWhenActiveBmagicMagic10603_801EEF24 and
 * stepWorkRecordAngleProjectPositionPairAndCountDownByte9OnFrameParityThenClear
 * RecordAtZeroBmagicMagic10603_801EF418. */
extern u8 *volatile D_801F05F8; /* @source 0x801F05F8 @kind data */

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt row
 * D_801EB4E0 = 0x801EB4E0, so no target-map row is needed. The two position
 * seeds read the word fields 0x34 and 0x38 and copy the word at 0x3C, and the
 * second seed reuses the loaded pointer for both reads the way the original
 * does. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Base of the scratchpad radius cell. This is the one scratchpad address the
 * function keeps live across the two rotator calls: the original sets the base
 * up once (lui $s0,0x1F80 / addiu $s0,$s0,0) and then reaches the cell as
 * 0x0($s0) for the seed store, the two signed loads that scale the position
 * pair and the unsigned load that copies the value into the record, so the
 * source keeps a byte view of the base in a pointer local. The cell is always
 * non-negative (0x60 through 0xD0), so only the load widths matter. Splat owns
 * the address through this target's map row D_1F800000 = 0x1F800000. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind unknown */

/* Scratchpad halfword beside it that carries the seeded angle. The original
 * re-materialises this address at each of the three uses (lui $at / sh,
 * lui $a0 / lh, lui $v0 / lhu), so the base is never cached and every access
 * names the symbol directly. Splat owns the address through this target's map
 * row D_1F800004 = 0x1F800004, which this lane added beside D_1F800000 because
 * no remaining asm span of the target references it. */
extern u16 D_1F800004; /* @source 0x1F800004 @kind unknown */

/* Rotates the halfword angle argument and returns the scaled step word. */
extern s32 func_801782FC(s32 angle);

/* Rotates the same halfword angle into the second axis and returns that scaled
 * step word. */
extern s32 func_801783C8(s32 angle);

/* @source 0x801EEFAC
 * @behavior Overlay work-step handler of the record published at the pointer
 *           cell 0x801F05F8, reached as entry 0 of the four-word handler run at
 *           0x801EF9C4 that the overlay's byte-2 dispatcher
 *           dispatchWorkByte2HandlerThenDrawAndPopWhenActiveBmagicMagic10603_801EEF24
 *           indexes with the record's byte 0x02, so it has no jal caller. It
 *           starts by decrementing the record's unsigned byte 0x0A and
 *           returning while the masked result is non-zero; the 0x18-byte frame
 *           keeps $ra and $s0 from the first instruction on, because the seed
 *           path calls three times.
 *
 *           When the countdown reaches zero it re-reads the cell once for the
 *           seed. Record byte 0x04 splits into the two scratchpad cells the
 *           overlay's step handlers consume: the scratchpad radius cell
 *           0x1F800000 takes (byte 0x04 >> 5) * 0x10 + 0x60 and the
 *           scratchpad angle cell 0x1F800004 takes (byte 0x04 & 0x1F) * 0x80,
 *           so the byte pairs a 32-entry direction sector with a distance band.
 *           The sector value is the argument of the first rotator
 *           func_801782FC and the second pointer-cell read stores the shared
 *           battle word 0x34 plus that rotator result times the signed
 *           scratchpad radius into the record word 0x10. func_801783C8 receives
 *           the signed scratchpad angle, and the third pointer-cell read stores
 *           the shared battle word 0x38 plus the second rotator result times the
 *           same signed radius into the record word 0x14, copies the shared
 *           battle word 0x3C into the record word 0x18 and publishes the
 *           unsigned scratchpad angle into the record halfword 0x0E.
 *
 *           The remaining seed writes re-read the pointer cell for every store,
 *           because the preceding store through the loaded pointer may alias it:
 *           record bytes 0x05, 0x06 and 0x07 take (rand() & 7) + 5, byte 0x03 and
 *           byte 0x09 are cleared, byte 0x0A takes (rand() & 3) << 3 and byte
 *           0x0B takes the unmasked rand() result. One final pointer-cell read
 *           copies the unsigned scratchpad radius into the record halfword 0x0C
 *           and increments the record byte 0x02 by one, which is the handler
 *           index the dispatcher reads, advancing the record from this seed step
 *           to the run's next handler.
 *
 *           Takes no arguments, returns nothing and reads no state outside the
 *           pointer cell, the shared battle work pointer, the two scratchpad
 *           cells and rand(). The body is not byte-identical to any other bmagic
 *           payload: the relocation-different copies of this handler at
 *           emi/bmagic/magic021/03 0x801EF600, emi/bmagic/magic125/03 0x801EFCE4
 *           and emi/bmagic/magic063/03 0x801EF40C are all still asm spans, so
 *           they are shape evidence only and this source was seeded from the
 *           original bytes, not cloned.
 * @status partial
 * @match 77.68
 * @residual Retained seed: 87 of 112 instructions and 440 of 448 bytes. Every
 *           statement, store order, load width, delay-slot filler and operand
 *           kind of the original is reproduced; the remaining delta is the
 *           register that carries the seed path's record pointer. The original
 *           keeps it in $v1 from the pointer cell's second read (0x801EEFDC)
 *           through the third read (0x801EF040) and leaves $a0 free for the
 *           rotator argument, whose shift it then computes a second time
 *           (sll $a0,$v0,7 before sll $v0,$v0,7) and leaves a nop in the
 *           func_801782FC delay slot. This build allocates that record-pointer
 *           quantity to $a0, so the two position stores, the angle store and
 *           the battle-pointer reloads that follow are $a0 forms and the
 *           rotator argument reaches $a0 through the move that fills the delay
 *           slot; the second sll is not emitted. Measured clean-C shapes for
 *           that one allocation: cells as byte anchors with FIELD_REF widths
 *           (65/112), the retained u16 cell declarations with per-access
 *           symbol folding (81/112), a duplicate angle expression (83/112), the
 *           same with the countdown read through the cell instead of a local
 *           (87/113), assignment-inside-the-argument store (retained, 87/112),
 *           volatile views of the radius and angle cells (79/116 and 87/112),
 *           separate pointer locals per cell read (76/119), a non-volatile
 *           pointer cell with a single read (54/112), the spad setup before the
 *           record read (87/112), a split in-place angle shift (87/112), a
 *           masked-byte local feeding two shift expressions (85/112), and
 *           volatile views of the same cells (79/116). None moves the
 *           quantity off $a0, so the original's allocation source shape stays
 *           unrecovered. No layout, flag, compiler or binding change is
 *           implied.
 */
void countDownWorkByteAThenSeedWorkRadiusAngleRandomTripletAndAdvanceWorkByte2BmagicMagic10603_801EEFAC(void) {
  u8 *record;
  u16 *spad;
  s32 angle;
  s32 step;

  if (--FIELD_REF(u8, D_801F05F8, 0x0A) == 0) {
    record = D_801F05F8;
    spad = &D_1F800000;
    *spad = (u16)(((FIELD_REF(u8, record, 0x04) >> 5) << 4) + 0x60);
    angle = (FIELD_REF(u8, record, 0x04) & 0x1F) << 7;
    step = func_801782FC(D_1F800004 = (u16)angle);
    record = D_801F05F8;
    FIELD_REF(u32, record, 0x10) =
        FIELD_REF(u32, D_801EB4E0, 0x34) +
        step * (s32)(s16)*spad;
    step = func_801783C8((s32)(s16)D_1F800004);
    record = D_801F05F8;
    FIELD_REF(u32, record, 0x14) =
        FIELD_REF(u32, D_801EB4E0, 0x38) +
        step * (s32)(s16)*spad;
    FIELD_REF(u32, record, 0x18) = FIELD_REF(u32, D_801EB4E0, 0x3C);
    FIELD_REF(u16, record, 0x0E) = D_1F800004;
    FIELD_REF(u8, D_801F05F8, 0x05) = (rand() & 7) + 5;
    FIELD_REF(u8, D_801F05F8, 0x06) = (rand() & 7) + 5;
    FIELD_REF(u8, D_801F05F8, 0x07) = (rand() & 7) + 5;
    FIELD_REF(u8, D_801F05F8, 0x03) = 0;
    FIELD_REF(u8, D_801F05F8, 0x09) = 0;
    FIELD_REF(u8, D_801F05F8, 0x0A) = (u8)((rand() & 3) << 3);
    FIELD_REF(u8, D_801F05F8, 0x0B) = (u8)rand();
    record = D_801F05F8;
    FIELD_REF(u16, record, 0x0C) = *spad;
    FIELD_REF(u8, record, 0x02)++;
  }
}

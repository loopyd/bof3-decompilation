#include "bof3/bof3.h"

/* Record pointer cell at 0x801F21F0, the last word of this payload. It holds
 * the 0x2C-byte bmagic work record that is being stepped: the overlay frame
 * loop at 0x801EEC28 publishes the selected record into this cell with one
 * `lui $at, %hi(D_801F21F0)` / `sw $v0, %lo(D_801F21F0)($at)` pair at
 * 0x801EECC8. Every access below re-reads the cell, so the original
 * re-materialises the cell address before each access instead of keeping the
 * record pointer live across a store. The cell sits inside the payload's
 * trailing `textbin` row labelled D_801F05A4, which no map row names, so the
 * reference binds through the target's generated undefined-symbol table. */
extern u8 *D_801F21F0; /* @source 0x801F21F0 @kind data */

/* Scratchpad halfword cell. The original addresses it as `lui $at,0x1F80` plus
 * `sh $v0,0x6($at)`, the absolute-constant store form the compiler emits for a
 * direct assignment to a fixed-cell symbol. */
extern u16 D_1F800006; /* @source 0x1F800006 @kind data */

/* Shared per-frame counter in main RAM, outside this overlay's image, bound
 * through this target's generated undefined_syms_auto.txt. Its low two bits
 * are the four-frame phase this span gates the record byte-6 step on, and the
 * same cell is halved by the draw helpers of the dispatcher at 0x801EF768. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image,
 * bound through this target's generated undefined_syms_auto.txt. The exact
 * lift decrementWorkByte9ByTwoIncrementWorkByteAOnFrameParityThenDecrement
 * GlobalWorkByteBAndClearWorkBytes0To4AtOneBmagicMagic09203_801EFA24 reads the
 * same record through this pointer and decrements the same byte 0xB. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared phase sampler: it returns the sample of a 12-bit phase in the same
 * fixed-point scale as the `>> 12` product below. */
s32 func_801782FC(s32 arg0);

/* Record-clearing helper of this target, exact at 0x801F0454. */
extern void clearRecordBytes0Through4BmagicMagic07303_801F0454(
    void); /* @source 0x801F0454 */

/* @source 0x801EFB14
 * @behavior Step handler of this overlay's stage walk, the third of the three
 *           code pointers at 0x801EEC1C (0x801EF864, 0x801EFA64, 0x801EFB14)
 *           that the dispatcher at 0x801EF768 copies into its own frame and
 *           indexes with the work record byte at offset 2, so it runs at step
 *           index 2. The word at 0x801EEC24 is the only word in the payload
 *           that names this address and no `jal` does, so that table is its
 *           only route. It steps the work record published through the pointer
 *           cell at 0x801F21F0: the record's unsigned halfword at 0xC is
 *           incremented, stored back unmasked, and the low six bits of the
 *           incremented value shifted left 6 are stored in the scratchpad
 *           halfword cell at 0x1F800006, the overlay family's phase-index
 *           publishing idiom. The same shifted value is passed to
 *           func_801782FC, and that call's result, scaled by 0x18 and shifted
 *           right 12 with an arithmetic shift, is added to the record halfword
 *           at 0x8 and stored in the record halfword at 0x20. The cell is then
 *           re-read and the record halfword at 0xA is added into the record
 *           halfword at 0x22. After that store, and only when the low two bits
 *           of the shared frame counter at 0x80143E6C are clear, the cell is
 *           re-read and the record byte at 0x6 is decremented at byte width.
 *           Finally the cell is re-read once more, the record byte at 0x5 is
 *           decremented at byte width, and when that countdown reaches zero it
 *           reads the shared battle work record pointer at 0x801EB4E0,
 *           decrements that record's byte 0xB at byte width, and calls the
 *           record-clearing helper at 0x801F0454 with no arguments. Takes no
 *           arguments, returns nothing, keeps a 0x18-byte frame only for $ra,
 *           and reads or writes no other state. The roles of work bytes 5, 6,
 *           0xC, 0x20 and 0x22 and of record byte 0xB are not established, so
 *           they stay offset-anchored. The nineteen instructions through the
 *           `sra $v1,$v1,12` are byte-identical to the exact lift
 *           advanceRecordHalfwordCAndAccumulateSampleIntoHalfwords20And22Then
 *           AdvanceByte2AtByte6LimitBmagicMagic07303_801EFA64 of this target,
 *           which supplied the head's statement order and operand forms; the
 *           byte-5 countdown and the record decrement follow the exact lift
 *           decrementWorkByte9ByTwoIncrementWorkByteAOnFrameParityThenDecrement
 *           GlobalWorkByteBAndClearWorkBytes0To4AtOneBmagicMagic09203_801EFA24,
 *           whose operand forms for the shared cells match this span. A
 *           comparison of this 228-byte span against every extracted payload
 *           under out/binaries, with both call words masked, found no twin, so
 *           no sibling is a clone source.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceRecordHalfwordCAndAccumulateSampleIntoHalfwords20And22ThenDecrementByte6EveryFourthFrameDecrementByte5ClearRecordAtZeroBmagicMagic07303_801EFB14(
    void) {
  u16 phase;
  s32 sample;
  u8 count;

  phase = ((u16 *)D_801F21F0)[6] + 1;
  ((u16 *)D_801F21F0)[6] = phase;
  D_1F800006 = (phase & 0x3F) << 6;
  sample = func_801782FC((phase & 0x3F) << 6);
  ((u16 *)D_801F21F0)[0x10] = ((u16 *)D_801F21F0)[4] + ((sample * 0x18) >> 12);
  ((u16 *)D_801F21F0)[0x11] += ((u16 *)D_801F21F0)[5];
  if ((D_80143E6C & 3u) == 0) {
    ((u8 *)D_801F21F0)[6]--;
  }
  count = ((u8 *)D_801F21F0)[5] - 1;
  ((u8 *)D_801F21F0)[5] = count;
  if (count == 0) {
    D_801EB4E0[0x0B]--;
    clearRecordBytes0Through4BmagicMagic07303_801F0454();
  }
}

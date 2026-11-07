#include "bof3/bof3.h"

/* Record pointer cell at 0x801F17B4 in this overlay's trailing read-only block.
 * It holds the 0x2C-byte work record that is being stepped: the overlay frame
 * loop at 0x801EEC28 publishes the selected entry of the 0x80-entry array based
 * at 0x801F01B4 into this cell with one `sw` at 0x801EECE8. Every access below
 * re-reads the cell, so the original re-materializes the cell address before
 * each access instead of keeping the record pointer live. The cell sits inside
 * the reviewed trailing `textbin` row labelled 0x801F00BC, which no map row
 * names, so the reference binds through the target's generated undefined-symbol
 * table. Every access reads the cell directly, so the pointer is never held in a
 * live variable across a store; that spelling is what reproduces the original's
 * three separate cell re-materializations and its register choices. */
extern u8 *D_801F17B4; /* @source 0x801F17B4 @kind data */

/* Scratchpad halfword cell. The original addresses it as `lui $at,0x1F80` plus
 * `sh $v0,0x6($at)`, the absolute-constant store form the compiler emits for a
 * direct assignment to a fixed-cell symbol. */
extern u16 D_1F800006; /* @source 0x1F800006 @kind data */

s32 func_801782FC(s32 arg0);

/* @source 0x801EF67C
 * @behavior Step handler holding word 7 of this overlay's entry stage table:
 *           the nine-word pointer run at 0x801EEC04 carries 0x801EF67C at
 *           offset 0x1C, and no `jal` in the payload names this address, so the
 *           table is the only route to it. It steps the work record published
 *           through the pointer cell at 0x801F17B4: the record's unsigned
 *           halfword at 0xC is incremented, stored back unmasked, and the low
 *           six bits of the incremented value shifted left 6 are stored in the
 *           scratchpad halfword cell at 0x1F800006, the overlay family's
 *           phase-index publishing idiom. The same shifted value is passed to
 *           func_801782FC, and that call's result, scaled by 0x18 and shifted
 *           right 12 with an arithmetic shift, is added to the record halfword
 *           at 0x8 and stored in the record halfword at 0x20. The cell is then
 *           re-read and the record halfword at 0xA is added into the record
 *           halfword at 0x22. Finally the record byte at 0x6 is incremented at
 *           byte width, and when the incremented byte equals the record byte at
 *           0x7 the record byte at 0x2 is incremented. Takes no arguments,
 *           returns nothing, keeps a 0x18-byte frame only for $ra, and reads or
 *           writes no other state. The 176-byte span is byte-identical to the
 *           still-unlifted 0x801EF67C of emi/bmagic/magic071/03 and
 *           emi/bmagic/magic072/03, confirmed by a payload compare, so neither
 *           offers a source to clone; this body was seeded from the original
 *           bytes and its shapes from the scenario phase-publisher idiom.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceRecordHalfwordCAndAccumulateSampleIntoHalfwords20And22ThenAdvanceByte2AtByte6LimitBmagicMagic07003_801EF67C(
    void) {
  u16 counter;
  s32 sample;
  u8 count;

  counter = ((u16 *)D_801F17B4)[6] + 1;
  ((u16 *)D_801F17B4)[6] = counter;
  D_1F800006 = (counter & 0x3F) << 6;
  sample = func_801782FC((counter & 0x3F) << 6);
  ((u16 *)D_801F17B4)[0x10] = ((u16 *)D_801F17B4)[4] + ((sample * 0x18) >> 12);
  ((u16 *)D_801F17B4)[0x11] += ((u16 *)D_801F17B4)[5];
  count = ((u8 *)D_801F17B4)[6] + 1;
  ((u8 *)D_801F17B4)[6] = count;
  if (count == ((u8 *)D_801F17B4)[7]) {
    ((u8 *)D_801F17B4)[2]++;
  }
}

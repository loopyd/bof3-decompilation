#include "bof3/bof3.h"

/*
 * Record pointer cell at 0x801F21F0, the last word of this payload. It holds
 * the 0x2C-byte bmagic work record that is being stepped: the overlay frame
 * loop at 0x801EEC28 publishes the selected record into this cell with one
 * `lui $at, %hi(D_801F21F0)` / `sw $v0, %lo(D_801F21F0)($at)` pair at
 * 0x801EECC8. Every access below re-reads the cell, so the original
 * re-materialises the cell address before each access instead of keeping the
 * record pointer live across a store. The cell sits inside the payload's
 * trailing `textbin` row labelled D_801F05A4, which no map row names, so the
 * reference binds through the target's generated undefined-symbol table.
 */
extern u8 *D_801F21F0; /* @source 0x801F21F0 @kind data */

/* Scratchpad halfword cell. The original addresses it as `lui $at,0x1F80` plus
 * `sh $v0,0x6($at)`, the absolute-constant store form the compiler emits for a
 * direct assignment to a fixed-cell symbol. */
extern u16 D_1F800006; /* @source 0x1F800006 @kind data */

/* Shared phase sampler: it returns the sample of a 12-bit phase in the same
 * fixed-point scale as the `>> 12` product below. */
s32 func_801782FC(s32 arg0);

/* @source 0x801EFA64
 * @behavior Step handler of this overlay's stage walk: the nine-word pointer
 *           run based at 0x801EEC04 carries 0x801EFA64 at offset 0x1C, and no
 *           `jal` in the payload names this address, so the only route to it is
 *           that table, reached through the three-word copy the dispatcher at
 *           0x801EF768 takes from 0x801EEC1C and indexes with the work record
 *           byte at offset 2; this is the middle entry. It steps the work
 *           record published through the pointer cell at 0x801F21F0: the
 *           record's unsigned halfword at 0xC is incremented, stored back
 *           unmasked, and the low six bits of the incremented value shifted
 *           left 6 are stored in the scratchpad halfword cell at 0x1F800006,
 *           the overlay family's phase-index publishing idiom. The same shifted
 *           value is passed to func_801782FC, and that call's result, scaled by
 *           0x18 and shifted right 12 with an arithmetic shift, is added to the
 *           record halfword at 0x8 and stored in the record halfword at 0x20.
 *           The cell is then re-read and the record halfword at 0xA is added
 *           into the record halfword at 0x22. Finally the record byte at 0x6 is
 *           incremented at byte width, and when the incremented byte equals the
 *           record byte at 0x7 the record byte at 0x2 is incremented. Takes no
 *           arguments, returns nothing, keeps a 0x18-byte frame only for $ra,
 *           and reads or writes no other state. The name keeps the spelling its
 *           peer-lifted twins use for this exact body: emi/bmagic/magic162/03
 *           0x801EF678 and emi/bmagic/magic070/03 0x801EF67C, both exact, from
 *           which this span differs in no instruction but the three address
 *           immediates of the record pointer cell, so neither is a clone
 *           source and both read as shape evidence only. emi/bmagic/magic071/03
 *           and emi/bmagic/magic072/03 hold the same shape at 0x801EF67C and
 *           their rows are still assembly. No byte-identical twin exists in the
 *           bmagic payload set.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceRecordHalfwordCAndAccumulateSampleIntoHalfwords20And22ThenAdvanceByte2AtByte6LimitBmagicMagic07303_801EFA64(
    void) {
  u16 counter;
  s32 sample;
  u8 count;

  counter = ((u16 *)D_801F21F0)[6] + 1;
  ((u16 *)D_801F21F0)[6] = counter;
  D_1F800006 = (counter & 0x3F) << 6;
  sample = func_801782FC((counter & 0x3F) << 6);
  ((u16 *)D_801F21F0)[0x10] = ((u16 *)D_801F21F0)[4] + ((sample * 0x18) >> 12);
  ((u16 *)D_801F21F0)[0x11] += ((u16 *)D_801F21F0)[5];
  count = ((u8 *)D_801F21F0)[6] + 1;
  ((u8 *)D_801F21F0)[6] = count;
  if (count == ((u8 *)D_801F21F0)[7]) {
    ((u8 *)D_801F21F0)[2]++;
  }
}

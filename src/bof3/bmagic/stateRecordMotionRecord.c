#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original folds the symbolic address
 * into the load (lui $v1,0x1F80 / lw $v1,0x44($v1)), and the exact same-target
 * sibling callWorkRecordHandler declares the
 * same cell for the same role. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Pointer cell at 0x801F28B4 holding the bmagic work record that is being
 * stepped. 0x801F009C stores &D_801F13B4[index * 0x1C] into this cell and calls
 * the dispatcher at 0x801F0448 immediately afterwards, and that dispatcher
 * reads the cell back before selecting an entry of its two-entry handler table,
 * whose entry 0 is this function. The cell is the last word of this overlay's
 * trailing data block, so the reference binds through the target's generated
 * undefined-symbol table. */
extern u8 *D_801F28B4; /* @source 0x801F28B4 @kind data */

/* Acting battler index byte in main RAM, read once per offset pair. The stores
 * that sit between the two reads go through the work record pointer, which may
 * alias this byte, so the original re-reads it and recomputes the 0x140-byte
 * stride. Its base is materialised once into $a0 and both reads use
 * lbu $v1,0($a0), which is why the object is addressed as a one-element row
 * rather than as a scalar: a scalar spelling let the value be carried across
 * the store and the address be folded into each load. */
extern u8 D_80146374[]; /* @source 0x80146374 @kind unknown */

/* Party record block in main RAM, addressed with a 0x140-byte stride. This body
 * reads only the byte at stride offset 0x00 and the two halfwords at 0x26 and
 * 0x28, and the original materialises each of those three bases separately
 * (its own lui %hi / addu / load %lo triple) instead of forming one record
 * pointer, so each is declared and addressed at its own address. */
extern u8 D_80145E98[]; /* @source 0x80145E98 @kind unknown */
extern u8 D_80145EBE[]; /* @source 0x80145EBE @kind unknown */
extern u8 D_80145EC0[]; /* @source 0x80145EC0 @kind unknown */

/* Signed byte pair table in this overlay's trailing read-only block, indexed by
 * the party record byte doubled: the original scales that byte with a single
 * sll by one and reads the pair at 0x801F138C plus that offset, low byte first
 * and high byte one past it. */
typedef struct BmagicMagic06403OffsetPair {
  s8 x;
  s8 y;
} BmagicMagic06403OffsetPair;

extern BmagicMagic06403OffsetPair D_801F138C[]; /* @source 0x801F138C @kind table */

/* @source 0x801F04A0
 * @behavior Seeds the motion fields of the bmagic work record and advances its
 * handler byte. Entry 0 of the two-entry handler table that the dispatcher at
 * 0x801F0448 selects with the unsigned record byte at offset 0x1: fills the
 * record halfword at 0x2 from the low twelve bits of rand(), writes the shared
 * work-object byte 0xB lifted into 16.16 form into the record words at 0xC and
 * 0x10 (that byte is read once per word, because the stores through the record
 * may alias the work object), stores the fixed-point velocities 0x19800 and
 * 0x32000 into the record words at 0x18 and 0x14, and then fills the record
 * words at 0x4 and 0x8 with a halfword of the acting battler's 0x140-byte party
 * record (offsets 0x26 and 0x28, selected by the shared index byte at
 * 0x80146374) plus the signed offset pair that the party record byte at offset
 * 0x00 selects from the 0x801F138C table. Finally it increments the record
 * handler byte at offset 0x1 so the next dispatch reaches entry 1, which
 * integrates the words at 0xC and 0x10 by the velocities at 0x14 and 0x18.
 * The roles of the record words at 0x4 and 0x8 and of the party record byte at
 * offset 0x00 remain unproven, so they stay offset-based. Takes no arguments
 * and returns nothing; the 0x18-byte frame only keeps $ra across the rand call.
 *
 * Representation notes: the rand result is held in a local before the two
 * pointer loads so the framed call runs before them and needs no callee-saved
 * register; loading the pointers first measured 15/65 with a 0x20-byte frame.
 * The four work-object-derived stores stay in ascending offset order, which is
 * the order the original shows. A scalar index byte measured 39/65, with the
 * multiplied value carried across the record store and the symbol address
 * folded into each load instead of being materialised once in $a0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateRecordMotionRecord(void) {
  u8 *record;
  u8 *work;
  s32 roll;
  s32 index;

  roll = rand();
  record = D_801F28B4;
  work = g_battle_work;
  *(u16 *)(record + 0x2) = roll & 0xFFF;
  *(s32 *)(record + 0xC) = work[0xB] << 16;
  *(s32 *)(record + 0x10) = work[0xB] << 16;
  *(s32 *)(record + 0x14) = 0x32000;
  *(s32 *)(record + 0x18) = 0x19800;
  index = D_80146374[0] * 0x140;
  *(s32 *)(record + 0x4) =
      *(s16 *)(D_80145EBE + index) + D_801F138C[D_80145E98[index]].x;
  index = D_80146374[0] * 0x140;
  *(s32 *)(record + 0x8) =
      *(s16 *)(D_80145EC0 + index) + D_801F138C[D_80145E98[index]].y;
  record[0x1]++;
}

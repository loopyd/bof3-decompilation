#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044. Declared as a named symbol so
 * every access is a lui plus offset load, which is how the original
 * re-materializes the cell before each access group at 0x801EF140, 0x801EF16C
 * and 0x801EF18C: a store through the loaded pointer may alias the cell, so each
 * group reads it again. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt,
 * which the remaining asm span func_801EF1C8 still references, so no target-map
 * row is needed. The exact bmagic siblings read the same record through
 * 0x801EB4E0. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF140
 * @behavior Entry 2 of this overlay's four-entry work-byte-1 handler table based
 *           at 0x801EF784 (0x801EEF34, 0x801EEFBC, this function, 0x801EF1C8),
 *           which the dispatcher at 0x801EEE90 indexes with the scratchpad work
 *           object's state byte 0x1; the entry word at 0x801EF78C is the only
 *           in-image reference, so the body has no jal caller. It reads the
 *           work-object pointer cell at 0x1F800044 and subtracts 0x60 from the
 *           unsigned halfword at 0x3E (lhu / addiu -0x60 / sh), holding 0x10 in
 *           one register for both byte comparisons. It then tests work byte 9 as
 *           a zero-extended copy of the loaded byte (andi 0xFF) and, when that
 *           byte is not 0x10, stores the loaded byte plus one at byte width; the
 *           addiu lands in the branch delay slot and the store is guarded. It
 *           re-reads the cell and adds two to work byte 0xA the same way while
 *           that byte is not 0x10, comparing the byte-wide load directly with no
 *           0xFF mask. The first two groups run through one work-object
 *           quantity, so both read the cell into the same register; the last
 *           group re-reads the cell into its own pointer local, which is what
 *           gives it the register the halfword compare leaves free. It loads the
 *           shared battle work record through 0x801EB4E0, compares the work
 *           object's signed halfword 0x3E with the record's signed halfword 0x3E
 *           and, when they are equal, increments work byte 0x2 as a byte-width
 *           read-modify-write so the handler advances to the next byte-0x2 step.
 *           Leaf with no frame; takes no arguments, returns nothing and reads or
 *           writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkDistanceBy60AdvanceWorkBytes9AndAUpTo10ThenAdvanceWorkByte2WhenDistanceMatchesRecordBmagicMagic15803_801EF140(void) {
  u8 *work;
  u8 *record;
  u8 *state;
  u8 byte9;
  u8 byteA;

  work = g_battle_work;
  *(u16 *)(work + 0x3E) -= 0x60;
  byte9 = work[9];
  if (byte9 != 0x10) {
    work[9] = byte9 + 1;
  }
  work = g_battle_work;
  byteA = work[0xA];
  if (byteA != 0x10) {
    work[0xA] = byteA + 2;
  }
  state = g_battle_work;
  record = D_801EB4E0;
  if (*(s16 *)(state + 0x3E) == *(s16 *)(record + 0x3E)) {
    state[2]++;
  }
}

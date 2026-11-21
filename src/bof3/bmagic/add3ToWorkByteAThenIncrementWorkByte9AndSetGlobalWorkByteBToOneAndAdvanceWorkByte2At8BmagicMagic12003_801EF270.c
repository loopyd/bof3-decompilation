#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's step handlers.
 * Kept as the named cell symbol bound in this target's map instead of the
 * constant-address macro: the symbolic address folds into each load
 * (lui $vN,0x1F80 / lw $vN,68($vN)), so each of this body's three access
 * groups re-materialises the base on its own, which is what the original
 * emits, while the macro's constant address let the compiler keep one full
 * address in a register. A store through the pointer may alias the cell, so
 * every group is a fresh read. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work pointer in main RAM, outside this overlay's image; Splat
 * binds the address through this target's generated undefined_syms_auto.txt,
 * so this lift needs no target-map row. The family keeps the role
 * address-anchored: its exact step handlers read and write byte 0xB of the
 * same record, and nothing in this payload names the record. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF270
 * @behavior Frame step of the overlay work object: adds 3 to the work object
 *           byte at offset 0xA with a byte-width read-modify-write (lbu /
 *           addiu 3 / sb), then reads the scratchpad pointer cell at
 *           0x1F800044 a second time and increments the work byte at offset
 *           0x9. The stored value is truncated with andi 0xFF and tested
 *           against 0x8, which the body materialises as its own constant and
 *           does not reload the byte to test. Only when that byte reaches
 *           0x8 does it set byte 0xB of the record pointed to by 0x801EB4E0
 *           to one, read the scratchpad cell a third time and increment the
 *           work object byte at offset 0x2, advancing the overlay's work
 *           byte 2 handler sequence. Leaf with no frame: takes no arguments,
 *           returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add3ToWorkByteAThenIncrementWorkByte9AndSetGlobalWorkByteBToOneAndAdvanceWorkByte2At8BmagicMagic12003_801EF270(void) {
  u8 *workA;
  u8 *work9;
  u8 value;

  workA = g_battle_work;
  workA[0xA] += 3;
  work9 = g_battle_work;
  value = work9[9] + 1;
  work9[9] = value;
  if (value == 8) {
    D_801EB4E0[0xB] = 1;
    g_battle_work[2]++;
  }
}

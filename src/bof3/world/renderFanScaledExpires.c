#include "bof3/bof3.h"

/* The overlay work record is published through the shared scratchpad pointer
 * cell at 0x1F800044, the same cell the other steps of this handler table use.
 * The cell is a plain pointer: the stores through the loaded record already
 * separate the reads, so no volatility is needed. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* The draw helper at 0x801F36DC builds a translucent gouraud fan of 32
 * triangles around (arg0, arg1) with its radius taken from arg2, reading the
 * primary cursor cell and the sin/cos helpers. It is still unlifted assembly
 * inside this target, so only its prototype is declared. */
void func_801F36DC(s16 arg0, s16 arg1, s16 arg2);

/* @source 0x801F3644
 * @behavior Overlay step handler, entry 2 of the local state-byte-1 handler
 * table D_801F460C at 0x801F460C. Reads the work record published at the
 * scratchpad pointer cell 0x1F800044 and calls the fan helper at 0x801F36DC
 * with the record's signed halfwords +0x2E and +0x30 as the fan center and
 * its byte +0x09 as the radius scale. It then decrements byte +0x09 and, when
 * that byte reaches zero, re-reads the cell and advances the record's state
 * byte +0x01 by one. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderFanScaledExpires(
    void) {
  u8* work;
  u8* record;

  work = D_1F800044;
  func_801F36DC(*(s16*)(work + 0x2e), *(s16*)(work + 0x30), work[0x09]);
  record = D_1F800044;
  if (--record[0x09] == 0) {
    record = D_1F800044;
    record[0x01]++;
  }
}

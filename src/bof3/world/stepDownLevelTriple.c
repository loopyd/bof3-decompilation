#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-reads the cell through
 * its own address load for every access group after the opening halfword/level
 * pair, so only those two opening accesses share one load, as a local pointer.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;
/* Pointer cell of the shared record whose halfword at 0x7E the non-reset arm of
 * this step consumes.
 * @source 0x80146884 @kind unknown
 */
extern u8* D_80146884;

/* @source 0x801F3FFC
 * @behavior Per-frame step of the scratch work record published through the
 * pointer cell at 0x1F800044: subtracts 0x14 from the record halfword at 0x3E,
 * then raises each of the signed level bytes 0x5D, 0x5E and 0x5F by 2 while it
 * reads below -0x40 - the first through the opening load of the pointer cell,
 * the other two each through their own fresh load of it. It then re-reads the
 * cell once and tests the signed halfword at 0x3E against 0xA00: while that
 * halfword reads 0xA00 it clears flag bit 0x20 of record byte 0, stores 0 into
 * work byte 0x5C and, through one further load of the cell, into the level
 * bytes 0x5F, 0x5E and 0x5D in that order; in the other arm it consumes two
 * units of the halfword at 0x7E of the record published through the global
 * pointer cell 0x80146884. Work byte 4 is untouched here, and this step clears
 * the flag bit and arms nothing, so it is the counterpart of the neighbouring
 * handler stateTripleLevelTriple,
 * which had measured as setting that flag bit, writing work byte 0x5C = 1 and
 * placing the level triple at -0x40. The payload's handler pointer run at
 * 0x801F6614 is the only word that names this entry and a data blob carries no
 * prototype, so no caller supplies argument roles: it takes no arguments,
 * calls nothing and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepDownLevelTriple(
    void) {
  u8* record;
  u8* levels;
  u8* work;

  record = D_1F800044;
  *(u16*)(record + 0x3e) -= 0x14;
  if (*(s8*)(record + 0x5d) < -0x40) {
    *(s8*)(record + 0x5d) += 2;
  }
  if (*(s8*)(D_1F800044 + 0x5e) < -0x40) {
    *(s8*)(D_1F800044 + 0x5e) += 2;
  }
  if (*(s8*)(D_1F800044 + 0x5f) < -0x40) {
    *(s8*)(D_1F800044 + 0x5f) += 2;
  }
  record = D_1F800044;
  if (*(s16*)(record + 0x3e) == 0xa00) {
    record[0] &= 0xdf;
    D_1F800044[0x5c] = 0;
    levels = D_1F800044;
    levels[0x5f] = 0;
    levels[0x5e] = 0;
    levels[0x5d] = 0;
  } else {
    work = D_80146884;
    *(u16*)(work + 0x7e) -= 2;
  }
}

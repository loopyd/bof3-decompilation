#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-reads the cell through
 * its own address load for every store group after the opening one, so only the
 * opening halfword store, the flag access and the level-triple group share a
 * local pointer.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;
/* Pointer cell of the shared work record whose byte 0x93 selects the 12-byte
 * counter record this step advances.
 * @source 0x80146884 @kind unknown
 */
extern u8* D_80146884;
/* Base of the 12-byte counter records selected by the work record's byte 0x93:
 * bytes 0x0, 0x1 and 0x2 are the three counters, and the original reaches the
 * second and third through their own generated symbols at 0x80145BD7 and
 * 0x80145BD8.
 * @source 0x80145BD6 @kind table
 */
extern u8 D_80145BD6[];
extern u8 D_80145BD7[];
extern u8 D_80145BD8[];
/* Pointer cell of the shared record whose halfword 0x0A this step consumes.
 * @source 0x80148210 @kind unknown
 */
extern u16* D_80148210;

/* @source 0x801F4B0C
 * @behavior Per-frame step of the scratch work record published through the
 * pointer cell at 0x1F800044: adds 8 to the record halfword at 0x3E, then, for
 * the 12-byte counter record selected by byte 0x93 of the work record published
 * through the global pointer cell 0x80146884, either adds 2 to each of the three
 * counter bytes 0x0, 0x1 and 0x2 - the first together with its guard read, the
 * other two each through a fresh read of 0x80146884 - or, once signed counter
 * byte 0x0 has reached 0x1E, sets flag bit 0x20 of record byte 0 and arms work
 * byte 0x5C = 1, the signed level triple at 0x5F/0x5E/0x5D = -0x40 and work byte
 * 4 = 1. Both arms then consume two units of the halfword at 0x0A of the record
 * published through the global pointer cell 0x80148210. Work byte 4 is the
 * selector the neighbouring dispatcher indexes its two-entry handler table with,
 * so this handler arms the state the following decrement handler steps down.
 * Takes no arguments, calls nothing and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkCounterTripleBy2OrArmScratchWorkMin40LevelTripleWorld04Area17413_801F4B0C(
    void) {
  s8 level;
  u32 offset;
  u8* record;
  u8* levels;
  u8* work;

  record = D_1F800044;
  /* The work-record pointer is read before the halfword step because the
   * original reads 0x80146884 ahead of that store: it keeps the cell pointer in
   * the register the halfword load cannot use, which is what lets the original
   * fill that load's delay slot and costs eight bytes when the read is left
   * after the store. */
  work = D_80146884;
  *(u16*)(record + 0x3e) += 8;
  offset = work[0x93] * 12;
  level = *(s8*)(D_80145BD6 + offset);
  if (level < 0x1e) {
    *(s8*)(D_80145BD6 + offset) = level + 2;
    offset = D_80146884[0x93] * 12;
    D_80145BD7[offset] += 2;
    offset = D_80146884[0x93] * 12;
    D_80145BD8[offset] += 2;
  } else {
    record[0] |= 0x20;
    D_1F800044[0x5c] = 1;
    /* One load of the cell serves the whole level triple. */
    levels = D_1F800044;
    *(s8*)(levels + 0x5f) = -0x40;
    *(s8*)(levels + 0x5e) = -0x40;
    *(s8*)(levels + 0x5d) = -0x40;
    D_1F800044[4] = 1;
  }
  D_80148210[5] -= 2;
}

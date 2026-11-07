#include "bof3/bof3.h"

/* Scratch work pointer cell: the original materialises the cell address once,
 * loads the pointer once and reuses it as the base of all three position
 * reads, so the fixed-address cell symbol keeps that single load. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F37A4
 * @behavior Work-record seeding for the overlay's 0x18-byte-stride record
 * table at 0x800E4800: it takes the record the caller claimed from that table
 * in $a0 and writes the overlay's default parameter set plus the work position
 * triple. The lead byte at offset 0 takes 1 - the claim flag the table's own
 * free-entry scan func_801F3800 tests and its reset func_801F385C clears - the
 * state byte at offset 1 is cleared, the countdown byte at offset 2 takes 0x20
 * and the byte at offset 3 takes 0x40. The record's words at offsets 0x04,
 * 0x08 and 0x0C take, unchanged, the words at 0x34, 0x38 and 0x3C of the
 * scratch work object published by the pointer cell at 0x1F800044 - the same
 * position triple the scenario and world overlay siblings copy into their own
 * records - and its halfword at 0x14 takes 0x80 while its byte at 0x16 takes
 * 7. Byte 0x17, which this overlay's per-record update func_801F38B0 turns
 * into the pair of angle terms it accumulates, is left to the caller. It takes
 * one argument, returns nothing, and reads no memory beyond the two objects
 * named here.
 *
 * Name evidence: this target's func_801F3800 scans the 0x20 entries of the
 * 0x18-byte-stride table at 0x800E4800 and returns the first whose lead byte
 * reads zero, and func_801F385C clears that same table with the same stride
 * and bound; both are this overlay's own neighbours of this address. The only
 * two call sites in the payload - jal 0x801F37A4 at 0x801F2EBC inside
 * func_801F2E88 and at 0x801F2FE4 inside func_801F2F98 - pass the pointer that
 * scan returned in $a0 and then overwrite the record halfword 0x14 and byte
 * 0x17, i.e. they treat this call as the record's default seeding. The record
 * field roles come from the update loop func_801F38B0, which walks the same
 * 0x18-byte stride and 0x20-entry bound: it reads the byte at 0x1 as the state
 * that selects its two update arms, decrements the countdown byte at 0x2 to
 * release the record (clearing the claim byte at 0), steps the byte at 0x3 by
 * -2 and the halfword at 0x14 by +8, and accumulates the words at 0x4 and 0xC.
 *
 * Shape evidence: the "seed a claimed record from the scratch work position
 * triple" shape is this repository's exact sibling family, e.g.
 * claimFreeWorkRecordSeedWorkPositionWorld03Area13413_801F48B0 (same
 * 0x800E4800 table, same 0x34/0x38 cell pair) and
 * seedClaimedWorkRecordFromWorkObjectScenarioScena0800_801F8B50; those records
 * use other strides and default sets, so they are shape evidence only and this
 * function's constants and offsets come from its own original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkRecordDefaultsAndWorkPositionWorld03Area13213_801F37A4(u8* record) {
  u8* work;

  work = D_1F800044;
  *(u32*)(record + 0x4) = *(u32*)(work + 0x34);
  *(u32*)(record + 0x8) = *(u32*)(work + 0x38);
  *(u32*)(record + 0xC) = *(u32*)(work + 0x3C);
  record[0] = 1;
  *(u16*)(record + 0x14) = 0x80;
  record[3] = 0x40;
  record[0x16] = 7;
  record[1] = 0;
  record[2] = 0x20;
}

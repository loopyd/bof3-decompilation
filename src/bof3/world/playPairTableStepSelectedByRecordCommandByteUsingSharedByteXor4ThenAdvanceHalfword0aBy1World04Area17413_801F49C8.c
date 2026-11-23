#include "bof3/bof3.h"

/* Shared world/state id of the main executable that indexes the world record
 * table below.
 * @source 0x80143F00 @kind unknown
 */
extern u16 D_80143F00;

/* World record table of the shared main-executable data segment, indexed by the
 * shared world/state id. Word 0x10 of the selected record is the table of
 * per-state byte-script pointers this handler walks.
 * @source 0x8017F974 @kind table
 */
extern void* D_8017F974[];

/* Pointer cell of the shared front record in the main executable. This handler
 * reads its byte +3 as the index of the per-state script pointer and its
 * halfword +0x0A as the byte offset of the current two-byte command, then
 * advances that halfword by one. The cell is re-read for that store, so it
 * materialises the cell again.
 * @source 0x80148210 @kind data
 */
extern u8* D_80148210;

/* Three-entry table of pointers at the end of this payload, one per pair table.
 * Every pointed-to table holds eight two-byte records, the shape the resident
 * helper 0x801F4A5C indexes with its second argument.
 * @source 0x801F6748 @kind table
 */
extern u8* D_801F6748[];

/* Shared main-executable record base whose first byte this handler flips with
 * 0x04 to form the step index it passes. The byte role stays anchored to this
 * offset.
 * @source 0x80145E98 @kind unknown
 */
extern u8 D_80145E98[];

/* Resident overlay helper of this payload: indexes the record table it is given
 * by twice its second argument, requests the first byte of that record through
 * func_8014D6B8 and copies the second byte into work byte 0x2A, storing the
 * argument back into work byte 0x08.
 * @source 0x801F4A5C
 */
extern void playPairTableStepStoreWorkByte8RequestFirstByteAndCopySecondByteToWorkByte2aWorld04Area17413_801F4A5C(
    u8* table, u32 index);

/* @source 0x801F49C8
 * @behavior State handler of this overlay, reached only through the code
 *           pointer word at 0x801F664C of the pointer run inside the payload
 *           textbin blob; no jal in the payload names it. It indexes the world
 *           record table 0x8017F974 with the shared world/state id 0x80143F00,
 *           takes that record word at offset 0x10 as a table of pointers, picks
 *           the entry with byte 0x03 of the front record published through the
 *           pointer cell 0x80148210, and adds that record halfword 0x0A as a
 *           byte offset, so the result addresses one two-byte command of a
 *           per-state byte script. The command first byte at offset 0x02
 *           selects one of the three pair tables of the pointer table
 *           0x801F6748, and the first byte of the shared record base 0x80145E98
 *           with bit 0x04 flipped is the step index into the eight two-byte
 *           records of that pair table: the resident helper 0x801F4A5C stores
 *           the index into work byte 0x08, requests the record first byte
 *           through func_8014D6B8 and copies the record second byte into work
 *           byte 0x2A. The handler then advances the front record halfword 0x0A
 *           by one, and it re-reads the pointer cell for that store, so the
 *           halfword it advances is the one the helper left behind. Unlike the
 *           neighbours at 0x801F4810 and 0x801F48EC it never compares work byte
 *           0x08 with the command byte, so it has no test and no branch. Takes
 *           no arguments and returns nothing; the 0x18-byte frame only keeps
 *           $ra at 0x10 across the helper call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void playPairTableStepSelectedByRecordCommandByteUsingSharedByteXor4ThenAdvanceHalfword0aBy1World04Area17413_801F49C8(
    void) {
  u32* states;
  u8* work;
  u8* script;
  u8* cursor;
  u16 offset;

  work = D_80148210;
  states = *(u32**)((u8*)D_8017F974[D_80143F00] + 0x10);
  script = (u8*)states[work[3]];
  offset = *(u16*)(work + 0xA);
  cursor = script + offset;
  playPairTableStepStoreWorkByte8RequestFirstByteAndCopySecondByteToWorkByte2aWorld04Area17413_801F4A5C(
      D_801F6748[cursor[2]], D_80145E98[0] ^ 4);
  *(u16*)(D_80148210 + 0xA) += 1;
}

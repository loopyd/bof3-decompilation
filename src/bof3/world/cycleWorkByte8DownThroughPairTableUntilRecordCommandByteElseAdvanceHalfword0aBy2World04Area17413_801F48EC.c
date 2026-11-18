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
 * halfword +0x0A as the byte offset of the current two-byte command, then moves
 * that halfword by two in either arm. The cell is re-read inside the taken arm,
 * so each of those accesses materialises the cell again.
 * @source 0x80148210 @kind data
 */
extern u8* D_80148210;

/* Scratchpad pointer cell holding the work record this overlay publishes. Work
 * byte 0x08 is the step index that is compared with the command byte and cycles
 * through eight values.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Three-entry table of pointers at the end of this payload, one per pair table.
 * Every pointed-to table holds eight two-byte records, the shape the resident
 * helper 0x801F4A5C indexes with its second argument.
 * @source 0x801F6748 @kind table
 */
extern u8* D_801F6748[];

/* Resident overlay helper of this payload: indexes the record table it is given
 * by twice its second argument, requests the first byte of that record through
 * func_8014D6B8 and copies the second byte into work byte 0x2A, storing the
 * argument back into work byte 0x08.
 * @source 0x801F4A5C
 */
extern void func_801F4A5C(u8* table, u8 index);

/* @source 0x801F48EC
 * @behavior Mirror state handler of this overlay: it walks the same per-state
 *           byte script as the handler at 0x801F4810 but moves the step byte
 *           down instead of up. The two spans were compared at seed time and
 *           differed in two words only, the signed immediate of the step-byte
 *           update and the absolute jump word, which shifted by the 0xDC delta
 *           between the spans, so they are one body with the direction
 *           reversed. This one is reached only through the code pointer word at
 *           0x801F6648 of the pointer run inside the payload textbin blob; no
 *           jal in the payload names it. It indexes the world record table
 *           0x8017F974 with the shared world/state id 0x80143F00, takes that
 *           record word at offset 0x10 as a table of pointers, picks the entry
 *           with byte 0x03 of the front record published through the pointer
 *           cell 0x80148210, and adds that record halfword 0x0A as a byte
 *           offset, so the result addresses one two-byte command of a per-state
 *           byte script. While the step byte 0x08 of the work record published
 *           through the scratchpad cell 0x1F800044 differs from the command
 *           first byte at offset 0x02, it moves the step byte down by one modulo
 *           eight and plays that step through the eight two-byte records of the
 *           pair table named by the command second byte at offset 0x03: the
 *           selector byte indexes the three-entry pointer table 0x801F6748, and
 *           the resident helper 0x801F4A5C stores the step back into work byte
 *           0x08, requests the record first byte through func_8014D6B8 and
 *           copies the record second byte into work byte 0x2A. That arm then
 *           steps the record halfword 0x0A back by two. When the step byte
 *           already equals the command byte the handler instead steps halfword
 *           0x0A forward by two, moving the script to its next command. The
 *           taken arm re-reads the pointer cell 0x80148210 for the command byte
 *           it plays and again for the step it consumes, and re-reads the
 *           scratchpad cell for the step it passes, so the step consumed is the
 *           one the helper left behind. Takes no arguments and returns nothing;
 *           the 0x18-byte frame only keeps $ra at 0x10 across the helper call,
 *           and the roles of work byte 0x2A, of the shared id and of the front
 *           record stay anchored to their offsets.
 * @status exact
 * @match 100.00
 * @residual none
 */
void cycleWorkByte8DownThroughPairTableUntilRecordCommandByteElseAdvanceHalfword0aBy2World04Area17413_801F48EC(
    void) {
  u32* states;
  u8* work;
  u8* scratch;
  u8* entry;
  u8* cursor;
  u16 offset;

  work = D_80148210;
  scratch = D_1F800044;
  states = *(u32**)((u8*)D_8017F974[D_80143F00] + 0x10);
  entry = (u8*)states[work[3]];
  offset = *(u16*)(work + 0xA);
  if (scratch[8] != entry[offset + 2]) {
    scratch[8] = (scratch[8] - 1) & 7;
    cursor = entry + *(u16*)(D_80148210 + 0xA);
    func_801F4A5C(D_801F6748[cursor[3]], D_1F800044[8]);
    *(u16*)(D_80148210 + 0xA) -= 2;
  } else {
    *(u16*)(work + 0xA) = offset + 2;
  }
}

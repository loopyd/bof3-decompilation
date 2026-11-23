#include "bof3/bof3.h"

/* Scratchpad pointer cell holding the work record this overlay publishes. This
 * body stores the step index into work byte 0x08 before the request and
 * re-reads the cell afterwards for the work byte 0x2A store, so the record
 * pointer does not stay live across the call.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Resource-request helper of the shared main executable at 0x8014D6B8. This
 * body passes it the first byte of the two-byte pair record the step selected,
 * which is the asset id it starts.
 */
void func_8014D6B8(u32 resource);

/* @source 0x801F4A5C
 * @behavior Resident step player of this overlay, called by the three
 *           pair-table state handlers of this payload at 0x801F4810,
 *           0x801F48EC and 0x801F49C8 and named by no pointer word. It takes
 *           the base of one of the three pair tables of 0x801F6748 and a step
 *           index, stores that index into work byte 0x08 of the work record
 *           published through the scratchpad pointer cell 0x1F800044, indexes
 *           the table by twice the index masked to a byte, requests the first
 *           byte of the selected two-byte record through the shared
 *           main-executable helper func_8014D6B8, then re-reads the scratchpad
 *           cell and copies the record second byte into work byte 0x2A. The
 *           step index therefore reaches the work record before the record
 *           byte is consumed, and both stored bytes come from the selected
 *           record. Takes two arguments and returns nothing; the 0x18-byte
 *           frame keeps $ra at 0x14 and $s0 at 0x10, the saved register holding
 *           the record address across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void audioStoreFirstSecond(
    u8* table, u32 index) {
  u8* work;
  u8* record;
  u8 offset;

  work = D_1F800044;
  offset = index * 2;
  work[8] = index;
  record = table + offset;
  func_8014D6B8(record[0]);
  work = D_1F800044;
  work[0x2A] = record[1];
}

#include "bof3/bof3.h"

/* Shared world/state id of the main executable that indexes the world record
 * table below.
 * @source 0x80143F00 @kind unknown
 */
extern u16 D_80143F00;

/* World record table of the shared main-executable data segment, indexed by the
 * shared world/state id. Word 0x10 of the selected record is the table of
 * per-state byte-script pointers this handler reads.
 * @source 0x8017F974 @kind table
 */
extern void* D_8017F974[];

/* Pointer cell of the shared front record in the main executable. This handler
 * reads its byte +3 as the index of the per-state script pointer and its
 * halfword +0x0A as the byte offset of the current two-byte command, then
 * advances that halfword by two. The cell is re-read for that store, so it
 * materialises the cell again.
 * @source 0x80148210 @kind data
 */
extern u8* D_80148210;

/* Scratchpad pointer cell holding the work record this overlay publishes. The
 * handler reads its halfword 0x58 as the unit count it lowers by two and writes
 * the command's second byte into its work byte 0x2A. It re-reads the cell after
 * the resource call, so the store goes through a fresh cell load.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Shared main-executable resource request: the first argument is the resource
 * id and the second one the mode or unit count it is requested with. Callers in
 * this tree pass constant pairs and payload table pairs.
 * @source 0x8014D6DC
 */
extern void func_8014D6DC(s32 resource, s32 mode);

/* @source 0x801F2E5C
 * @behavior State handler of this overlay, reached only through the code
 *           pointer word 0x801F426C, entry 1 of the thirteen-word dispatch run
 *           based at 0x801F4268 inside the payload's textbin blob; no jal in
 *           the payload names it. It indexes the world record table 0x8017F974
 *           with the shared world/state id 0x80143F00, takes that record's word
 *           at offset 0x10 as a table of pointers, picks the entry with byte
 *           0x03 of the front record published through the pointer cell
 *           0x80148210, and adds that record's halfword 0x0A to it as a byte
 *           offset, so the result addresses one two-byte command of a per-state
 *           byte script. It requests the command's first byte at offset 0x02 as
 *           a resource through the shared helper func_8014D6DC, passing the
 *           work record's halfword 0x58 lowered by two and masked to 0xFFFF as
 *           the mode, then re-reads the scratchpad cell 0x1F800044 and copies
 *           the command's second byte at offset 0x03 into work byte 0x2A.
 *           Finally it re-reads the pointer cell 0x80148210 and advances the
 *           front record's halfword 0x0A by two, moving the script to its next
 *           command. Takes no arguments and returns nothing; the 0x18-byte
 *           frame keeps $ra at 0x14 and $s0 at 0x10, the saved register holding
 *           the command's second byte across the resource call. The roles of the
 *           shared id and of the front record stay anchored to their offsets.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestResourceFromScript(
    void) {
  u32* states;
  u8* work;
  u8* script;
  u8* cursor;
  u16 offset;
  u8 next;

  work = D_80148210;
  states = *(u32**)((u8*)D_8017F974[D_80143F00] + 0x10);
  script = (u8*)states[work[3]];
  offset = *(u16*)(work + 0xA);
  cursor = script + offset;
  next = cursor[3];
  func_8014D6DC(cursor[2], (*(u16*)(D_1F800044 + 0x58) - 2) & 0xFFFFu);
  D_1F800044[0x2A] = next;
  *(u16*)(D_80148210 + 0xA) += 2;
}

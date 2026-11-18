#include "bof3/bof3.h"

void func_801172D0(void);

/* Published work object of the shared main executable, read through the
 * scratchpad pointer cell 0x1F800044. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* View of the spawn coordinate triple the work object carries at 0x34/0x38/0x3C,
 * the three words func_801172D0 copies into every record of the 0x800E4800 work
 * table. Fields are declared in the order the original stores them, not in
 * address order. */
typedef struct {
  u32 pad[13];
  u32 x;
  u32 y;
  u32 z;
} WorkPosition;

/* @source 0x80117048
 * @behavior Boot handler of this overlay: word 0 of the thirteen in-image
 * handler pointers the textbin row D_8011818C places, selected by
 * dispatchWorkByte1Table818cWorld01Area04913_80117004 while byte 1 of the work
 * object at the scratchpad pointer cell 0x1F800044 still reads 0. It stores the
 * spawn coordinate triple into that object, word 0x38 taking 0x548000, word
 * 0x34 taking 0x0B8000 and word 0x3C taking 0x01000000; those are the words
 * func_801172D0 and claimWorkRecordSeedWorkPositionWorld01Area04913_80117864
 * copy into the position triple at 0x04/0x08/0x0C of every 0x28-byte work
 * record and that the step handlers add their velocity words to, and read as
 * 16.16 fixed point they are 84.5, 11.5 and 256.0. It then calls func_801172D0,
 * which republishes the 0x800E4800 work table in D_801181DC and rewrites all
 * sixteen records from this object, giving each a rand() velocity pair at
 * 0x14/0x18, claim byte 0 = 1 and step byte 1 = 0. Finally it re-reads the
 * pointer cell and increments the object byte at 1 by one, so the state 1
 * handler clearWorkFlagsWhenNoEntryHandledWorld01Area04913_801170A4 runs from
 * the next frame on. The original materialises 0x0B8000 and 0x548000 ahead of
 * the pointer load, stores 0x38 before 0x34, and keeps the 0x01000000 store in
 * the call delay slot; the 0x18-byte frame only holds $ra at 0x10 across the
 * single call. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void bootHandlerSeedWorkCoordinateTripleResetRecordsThenAdvanceStateWorld01Area04913_80117048(void) {
  u32 x = 0xB8000;
  u32 y = 0x548000;
  u8* work;
  WorkPosition* pos;

  pos = (WorkPosition*)D_1F800044;
  pos->y = y;
  pos->x = x;
  pos->z = 0x1000000;
  func_801172D0();
  work = D_1F800044;
  work[1] = work[1] + 1;
}

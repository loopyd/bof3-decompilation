#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original keeps the pointer of its
 * opening load live through the halfword update and the state-byte store, so
 * the body reads the cell once.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Renderer of the work record, still on its assembled boundary.
 * @source 0x801F3958 @kind function
 */
extern void renderWorkRecord(void);

/* @source 0x801F377C
 * @behavior Reads the scratchpad work-object pointer at 0x1F800044, adds 0x10
 * to halfword 0x30 of that record and stores the sum back, then, once the
 * updated halfword read as a signed value has reached 0x80, advances work byte
 * 0x2 by one - the state selector the overlay's work-byte-2 dispatcher at
 * 0x801F344C indexes the local handler table 0x801F4FBC with, where this body
 * is entry 2 - and finally calls renderWorkRecord, which renders the record.
 * Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateAdvanceWorkCall3958(
    void) {
  u8* work;
  s32 offset;

  work = D_1F800044;
  offset = *(u16*)(work + 0x30) + 0x10;
  *(u16*)(work + 0x30) = offset;
  if ((s16)offset >= 0x80) {
    work[2]++;
  }
  renderWorkRecord();
}

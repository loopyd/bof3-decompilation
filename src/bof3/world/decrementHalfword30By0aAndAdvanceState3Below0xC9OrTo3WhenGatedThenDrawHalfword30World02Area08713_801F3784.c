#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* D_1F800044;
/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* The local emitter at 0x801F40F8 narrows both arguments with `sll`/`sra 16` in
 * its own prologue and this call site consumes no result. */
void func_801F40F8(s16 arg0, s16 arg1);

/* @source 0x801F3784
 * @behavior Handler 1 of the local work-byte-3 dispatch run based at
 *           0x801F5760, which the byte-3 dispatcher 0x801F36C0 indexes with
 *           record byte 0x03, over the work record published at the scratchpad
 *           pointer cell 0x1F800044. It subtracts 0x0A from the signed halfword
 *           at record offset 0x30 and stores the truncated halfword, then
 *           advances work state byte 0x03 while that stored value stays below
 *           0xC9. It writes 3 to state byte 0x03 through a second read of the
 *           pointer cell when the shared byte 0x801454F2 is nonzero and record
 *           byte 0x0B is nonzero, or the phase byte 0x80143BB0 is 2, or bit
 *           0x100 of halfword 0x80146258 is set. Finally it reads the cell once
 *           more and passes 0x5C and the signed halfword at 0x30 to the local
 *           emitter 0x801F40F8. It takes no arguments, uses no result and saves
 *           only the return address in a 0x18-byte frame. The only in-image word
 *           naming this address is 0x801F5764, entry 1 of that table. A payload
 *           scan with the `jal` immediate masked found nine sibling spans holding
 *           these same 0xD8 bytes, and the one at
 *           decrementHalfword30By0aAndAdvanceState3Below0xC9OrTo3WhenGatedThenDrawHalfword30World03Area12114_801F3768
 *           in emi/world03/area121/14 differed from this span in that single
 *           internal `jal` word alone, its bias equal to the 0x1C offset between
 *           the two spans, so the statement order came from that sibling with
 *           this overlay's own emitter address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword30By0aAndAdvanceState3Below0xC9OrTo3WhenGatedThenDrawHalfword30World02Area08713_801F3784(
    void) {
  u8* record;
  u8* emitter;
  s16* half;

  record = D_1F800044;
  half = (s16*)(record + 0x30);
  *half = (s16)(*half - 0x0A);
  if (*half < 0xC9) {
    record[3]++;
  }
  if ((D_801454F2 != 0 && D_1F800044[0x0B] != 0) ||
      D_80143BB0 == 2 || (D_80146258 & 0x100u) != 0) {
    D_1F800044[3] = 3;
  }
  emitter = D_1F800044;
  func_801F40F8(0x5C, FIELD_REF(s16, emitter, 0x30));
}

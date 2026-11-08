#include "bof3/bof3.h"

extern u8* D_1F800044;
extern u8 D_801454F2;
extern u8 D_80143BB0;
extern u16 D_80146258;

/* The local emitter at 0x801F40C8 narrows both arguments with `sll`/`sra 16`
 * in its own body and the call site consumes no result. */
void func_801F40C8(s16 arg0, s16 arg1);

/* @source 0x801F3754
 * @behavior Handler 5 of the eight-entry state-byte-2 handler table based at
 *           0x801F4F64, run by the byte-2 dispatcher 0x801F34CC, over the work
 *           record published at scratchpad pointer cell 0x1F800044. Subtracts
 *           0xA from the signed halfword at record offset 0x30 and stores the
 *           truncated halfword, then increments state byte 0x03 while that
 *           stored value stays below 0xC9. It writes 3 to state byte 0x03
 *           through a second read of the pointer cell when the shared byte
 *           0x801454F2 is set and record byte 0x0B is nonzero, or the phase byte
 *           0x80143BB0 is 2, or bit 0x100 of halfword 0x80146258 is set. Finally
 *           it reads the cell again and passes 0x5C and the signed halfword at
 *           0x30 to the local emitter at 0x801F40C8. Takes no arguments and
 *           returns nothing. The 0xD8 bytes are byte-identical to the exact
 *           decrementHalfword30By0aAndAdvanceState3Below0xC9OrTo3WhenGatedThenDrawHalfword30World03Area12114_801F3768
 *           in emi/world03/area121/14 apart from the low byte of the internal
 *           `jal` word that encodes that overlay's own emitter address, so this
 *           body follows the reviewed sibling statement order with this
 *           target emitter.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword30By0aAndAdvanceState3Below0xC9OrTo3WhenGatedThenDrawHalfword30World03Area11513_801F3754(
    void) {
  u8* record;
  u8* emitter;
  s16* half;

  record = D_1F800044;
  half = (s16*)(record + 0x30);
  *half = (s16)(*half - 0xA);
  if (*half < 0xC9) {
    record[3]++;
  }
  if ((D_801454F2 != 0 && D_1F800044[0x0B] != 0) || D_80143BB0 == 2 ||
      (D_80146258 & 0x100u) != 0) {
    D_1F800044[3] = 3;
  }
  emitter = D_1F800044;
  func_801F40C8(0x5C, FIELD_REF(s16, emitter, 0x30));
}

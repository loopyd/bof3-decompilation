#include "bof3/bof3.h"

extern u8* D_1F800044;
extern u8 D_801454F2;
extern u8 D_80143BB0;
extern u16 D_80146258;

/* The local emitter at 0x801F40C8 narrows both arguments with `sll`/`sra 16`
 * in its own body and the call site consumes no result. */
void func_801F40C8(s16 arg0, s16 arg1);

/* @source 0x801F392C
 * @behavior Adds 0xA to the halfword at offset 0x30 of the work record
 *           published at scratchpad pointer cell 0x1F800044 and stores the
 *           truncated halfword there, then clears state byte 0x03 once that
 *           stored signed halfword reaches 0xF0. Unless the shared byte
 *           0x801454F2 is set, the phase byte 0x80143BB0 is 2, or bit 0x100 of
 *           halfword 0x80146258 is set, it writes 1 to state byte 0x03 through
 *           a second read of the pointer cell. Finally it reads the cell again
 *           and passes 0x5C and the signed halfword at 0x30 to the local
 *           emitter at 0x801F40C8. Takes no arguments and returns nothing.
 *           Reached as word 7 of the eight-word handler run based at
 *           0x801F4F64, which the state-byte-2 dispatcher 0x801F34CC indexes by
 *           state byte 0x02, and as word 3 of the run from 0x801F4F74 that the
 *           state-byte-3 dispatcher 0x801F3690 indexes. The 0xB0 bytes differ
 *           from those at 0x801F3940 in emi/world03/area121/14 in exactly one
 *           word, the internal `jal` that encodes each overlay's own emitter
 *           address (0x801F40C8 here, 0x801F40DC there), so this body follows
 *           that sibling's statement order with this target's emitter.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateUnlessGatedDraw(
    void) {
  u8* record;
  u8* emitter;
  s16* half;

  record = D_1F800044;
  half = (s16*)(record + 0x30);
  *half = (s16)(*half + 0x0A);
  if (*half >= 0xF0) {
    record[3] = 0;
  }
  if (D_801454F2 == 0 && D_80143BB0 != 2 && (D_80146258 & 0x100u) == 0) {
    D_1F800044[3] = 1;
  }
  emitter = D_1F800044;
  func_801F40C8(0x5C, FIELD_REF(s16, emitter, 0x30));
}

#include "bof3/bof3.h"

/* Work record published through the scratchpad pointer cell at 0x1F800044. The
 * four reads the body needs come back as `lui`/`lw` pairs through this symbol,
 * while the macro spelling of the same cell folds the address into one base
 * register and diverges. */
/* @source 0x1F800044 @kind data */
extern u8* scratchpadWorkRecord;

/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* The local emitter at 0x801F4034 narrows both arguments with `sll`/`sra 16`
 * in its own body and the call site consumes no result. */
void func_801F4034(s16 arg0, s16 arg1);

/* @source 0x801F36C0
 * @behavior Entry 1 of the four-entry work-byte-3 handler table based at
 *           0x801F5458, reached through the byte-3 dispatcher at 0x801F35FC.
 *           Subtracts 0x0A from the signed halfword at offset 0x30 of the work
 *           record published at the scratchpad pointer cell 0x1F800044 and
 *           stores the truncated halfword, then increments state byte 0x03
 *           while that stored value stays below 0xC9. It writes 3 to state byte
 *           0x03 through a second read of the pointer cell when the shared byte
 *           0x801454F2 is set and record byte 0x0B is nonzero, or the phase
 *           byte 0x80143BB0 is 2, or bit 0x100 of halfword 0x80146258 is set.
 *           Finally it reads the cell once more and passes 0x5C and the signed
 *           halfword at 0x30 to the local emitter at 0x801F4034. Takes no
 *           arguments, uses no result and saves only the return address in a
 *           0x18-byte frame.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The 0xD8 bytes at payload offset 0xAC0 are byte-identical to the exact
 * decrementHalfword30By0aAndAdvanceState3Below0xC9OrTo3WhenGatedThenDrawHalfword30World02Area08713_801F3784
 * span of emi/world02/area087/13 apart from the single `jal` word at body
 * offset 0xC0, whose target shifts by exactly the 0xC4 offset between the two
 * spans, and the same equality holds against the exact
 * decrementHalfword30By0aAndAdvanceState3Below0xC9OrTo3WhenGatedThenDrawHalfword30World03Area12114_801F3768
 * span, whose one differing word is that same `jal` and names that overlay's
 * own emitter. Each comparison found that word alone differing, so the family
 * statement order, operand forms, offset spelling and declaration style
 * transfer whole with this overlay's emitter address. The only in-image word
 * naming this address is 0x801F5464, entry 1 of the handler table 0x801F5458.
 */
void decrementGatedDrawHalfword30(
    void) {
  u8* record;
  u8* emitter;
  s16* half;

  record = scratchpadWorkRecord;
  half = (s16*)(record + 0x30);
  *half = (s16)(*half - 0x0A);
  if (*half < 0xC9) {
    record[3]++;
  }
  if ((D_801454F2 != 0 && scratchpadWorkRecord[0x0B] != 0) ||
      D_80143BB0 == 2 || (D_80146258 & 0x100u) != 0) {
    scratchpadWorkRecord[3] = 3;
  }
  emitter = scratchpadWorkRecord;
  func_801F4034(0x5C, FIELD_REF(s16, emitter, 0x30));
}

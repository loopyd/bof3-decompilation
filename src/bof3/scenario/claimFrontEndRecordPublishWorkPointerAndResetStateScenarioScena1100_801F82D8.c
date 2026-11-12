#include "bof3/bof3.h"

/* Front-end record table of the shared main executable: 20 records of 0x74
 * bytes, the stride the original scales this function's argument by. The
 * generated undefined_syms_auto.txt row binds the name, and the sibling plchar
 * lifts of the same table spell it the same way
 * (`D_80143FC8[(u32)work[0x0B] * 0x74u]`). */
extern u8 D_80143FC8[];

/* Scratchpad work-record pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, which is the form this span encodes before
 * each store group and after the helper call; the fixed-address lvalue form of
 * the same cell (SPAD_PTR_SLOT(u8, 0x44u)) kept the cell address in the
 * callee-saved $s0 across the call and measured 30/60 instructions at 220
 * bytes, first difference a `sw $s0, 0x10($sp)` the original does not have. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared main-executable lookup helper: applies the main lookup record whose id
 * matches its argument to the active work record, so the argument here is a
 * record id. This target's generated undefined_funcs_auto.txt row binds the
 * address-anchored name. */
void func_8014DD3C(s32 entry); /* @source 0x8014DD3C */

/* Resource request helper of the main executable, reached through the same
 * generated undefined_funcs_auto.txt row; the sibling boss and bmagic lifts
 * declare the same prototype and pass a resource id. */
void func_8014D6B8(u32 resource); /* @source 0x8014D6B8 */

/* @source 0x801F82D8
 * @behavior Claims a front-end record and makes it the active work record:
 *           masks the byte argument, scales it by the 0x74-byte record stride
 *           over the shared table D_80143FC8 (0x80143FC8), and publishes
 *           that record pointer into the scratchpad pointer cell 0x1F800044,
 *           which this overlay's dispatchers read as the work record. It then
 *           runs the shared lookup helper func_8014DD3C with record id 0x18 and
 *           resets the claimed record: byte 0x24 = 0, byte 0x2A = 0, byte 0x2B
 *           advanced by one, byte 0x48 = 0, the signed tint triplet at bytes
 *           0x5D..0x5F cleared, word 0x14 = 0 and word 0x20 = -8. One cell load
 *           feeds the three stores at 0x5D, 0x14 and 0x20; every other store
 *           re-reads the cell, because a store through the loaded pointer may
 *           alias the cell itself. The tint triplet clears before the resource
 *           request: byte 0x5E is cleared through its own cell read, which sits
 *           ahead of the two word stores in the emitted order while its store
 *           follows them, and the byte 0x5F clear directly before the call is
 *           the store the original keeps in the jal delay slot. The function
 *           requests resource 0x0E through func_8014D6B8 and finishes by
 *           storing 3 in the record's byte 1 and 1 in its byte 2. The 0x18-byte
 *           frame only keeps $ra at 0x10, nothing is returned, and every cell
 *           load stays in a call-clobbered register.
 *
 *           Representation notes from the gate: the fixed-address lvalue form
 *           of the cell measured 30/60 instructions at 220 bytes with the extra
 *           saved register above; the named cell with the 0x5E store left inline
 *           after the word stores measured 50/60 at the original 240 bytes, first
 *           difference a group reload in $v0 where the original builds $v1; the
 *           named cell with the 0x5E read ahead of the word stores measured 54/60
 *           and moved the call constant into that load's delay slot; placing the
 *           0x5F clear before the call filled the jal delay slot and matched.
 *
 *           Byte 1 is the position this overlay's handler dispatcher
 *           dispatchWorkByte1HandlerThenPublishMode2ScenarioScena1100_801F70B4
 *           reads (lbu 0x1 then the scaled load from the run at 0x801FAEC0).
 *           Word 3 of that run is byte data, not a code word, so the 3 stored
 *           here does not name an entry of that run and no handler is claimed.
 *           Byte and word roles above 0x24 are the offsets the battle work
 *           record shares with this table's records
 *           (include/bof3/battle/battle03_internal.h: +0x14/+0x20 words, +0x24
 *           kind byte, +0x2A/+0x2B script and step bytes, +0x5D..+0x5F tint
 *           triple), so they stay address-anchored.
 *
 *           The boundary is proven by the 0x18-byte frame at 0x801F82D8 and the
 *           `jr $ra` at 0x801F83C0; the only in-image reference is the `jal` at
 *           0x801F80E8, inside func_801F7F08, which passes scratchpad byte 0 as
 *           the index, and no pointer word in the payload names the span. A
 *           search of every payload under out/binaries for this 0xF0-byte body
 *           found no second copy, so the clone lever is declined and the body
 *           is seeded from this span's own bytes with the tint triplet, kind
 *           byte and step-byte statement order of the exact bmagic sibling
 *           seedWorkHeaderClearTintGroupAndRequestResourceThenAdvanceWorkByte2BmagicMagic17303_801F0540.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFrontEndRecordPublishWorkPointerAndResetStateScenarioScena1100_801F82D8(
    u8 index) {
  u8* work;
  u8* next;

  D_1F800044 = D_80143FC8 + index * 0x74u;
  func_8014DD3C(0x18);
  D_1F800044[0x24] = 0;
  D_1F800044[0x2A] = 0;
  D_1F800044[0x2B] += 1;
  D_1F800044[0x48] = 0;
  work = D_1F800044;
  work[0x5D] = 0;
  next = D_1F800044;
  *(u32 *)(work + 0x14) = 0;
  *(s32 *)(work + 0x20) = -8;
  next[0x5E] = 0;
  D_1F800044[0x5F] = 0;
  func_8014D6B8(0x0E);
  D_1F800044[0x1] = 3;
  D_1F800044[0x2] = 1;
}

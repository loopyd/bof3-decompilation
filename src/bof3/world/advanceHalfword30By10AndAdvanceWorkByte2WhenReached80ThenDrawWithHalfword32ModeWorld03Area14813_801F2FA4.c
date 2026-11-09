#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Overlay draw routine of this payload. Its one argument is a signed byte
 * mode: the body shifts it left by 24 before the SetShadeTex call, so the
 * routine consumes the low byte only. Declared here as an address-anchored
 * callee of this target. */
void func_801F31AC(s8 arg0);

/* @source 0x801F2FA4
 * @behavior Handler at word 2 of the state-byte-2 dispatch run based at
 *           0x801F4FC4, so it has no jal caller: the dispatcher at 0x801F2C5C
 *           reaches it through that table. Reads the work record pointer from
 *           the scratchpad cell 0x1F800044 once, adds 0x10 to the record
 *           halfword at +0x30, stores the truncated halfword back and advances
 *           the record dispatch byte at +2 by one when the signed halfword has
 *           reached 0x80. It then reloads the scratchpad pointer cell, reads
 *           the signed record halfword at +0x32 and calls the overlay draw
 *           routine at 0x801F31AC with mode -1 when that halfword is zero and
 *           mode 0 when it is not. Takes no arguments and returns nothing; its
 *           0x18-byte frame only holds ra at +0x10 and the record pointer load
 *           precedes the frame creation.
 *
 * Name evidence: the record byte at +2 is the index the state-byte-2
 * dispatcher of this overlay reads, which makes the single store to it an
 * advance of that dispatch state, and the record halfword at +0x30 is the
 * panel offset the neighbouring handlers of the same family step. The mode
 * argument comes from the record halfword at +0x32, so the draw call carries
 * the mode the bytes select.
 *
 * Shape evidence: a little-endian masked scan of every payload under
 * out/binaries, with call-target and internal branch words masked, finds this
 * 124-byte body only here and at 0x801F2F34 of emi/world03/area147/13, whose
 * single differing word is that call target. So no byte-identical twin exists
 * and the neighbour is shape evidence only. The word neighbours of the same
 * dispatch run differ in body: word 1 at 0x801F2E64 and word 3 at 0x801F3020
 * do not step this halfword, and the 0x801F5024 run holds handlers that draw
 * through 0x801F37B8 instead.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword30By10AndAdvanceWorkByte2WhenReached80ThenDrawWithHalfword32ModeWorld03Area14813_801F2FA4(
    void) {
  u8* record;
  u8* emitter;
  s16* half;

  record = D_1F800044;
  half = (s16*)(record + 0x30);
  *half = (s16)(*half + 0x10);
  if (*half >= 0x80) {
    record[2]++;
  }
  emitter = D_1F800044;
  if (*(s16*)(emitter + 0x32) != 0) {
    func_801F31AC(0);
  } else {
    func_801F31AC(-1);
  }
}

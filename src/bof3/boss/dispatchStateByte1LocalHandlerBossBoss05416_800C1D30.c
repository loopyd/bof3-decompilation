#include "bof3/bof3.h"

typedef void (*BossBoss05416Handler)(void);

/* The single handler this dispatcher seeds is the following function of this
 * overlay, 0x800C1D7C: it copies the three-word pointer run at 0x800C182C into
 * its own frame, calls the entry selected by work-record byte 2, and then runs
 * the two shared updates that bit 0 of the record's byte 0 selects. */
void func_800C1D7C(void);

/* @source 0x800C1D30
 * @behavior Overlay step dispatch on work-record byte 1: creates its 0x20-byte
 * frame, loads the work-record pointer published once at scratchpad pointer
 * cell 0x1F800044, seeds 0x800C1D7C into entry 0 of the two-entry handler array
 * at 0x10($sp) and calls the entry selected by that record's unsigned byte at
 * offset 1 with no arguments, keeping $ra at 0x18 across the indirect call.
 * Entry 1 of the frame array stays unseeded and the index is not bounded before
 * the scaled load, so only index 0 reads a seeded slot. It writes no memory
 * beyond its own frame and reads no other state. The 76-byte body is
 * instruction-for-instruction identical to the exact cross-target sibling
 * dispatchWorkByte1LocalHandlerBmagicMagic05203_801EEE94: frame, load and store
 * order, delay slots and the zero-argument indirect call are the same, and only
 * the lui/addiu pair that materializes the seeded handler differs, leaving 74 of
 * the 76 bytes equal, so that sibling supplies the shape and not the bytes. No
 * word of this payload holds this function's address and no jal reaches it, so
 * it is an overlay entry the main program calls directly; the prologue and the
 * return path place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte1LocalHandlerBossBoss05416_800C1D30(void) {
  BossBoss05416Handler handlers[2];

  handlers[0] = func_800C1D7C;
  handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}

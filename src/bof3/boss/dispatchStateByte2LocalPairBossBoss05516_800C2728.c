#include "bof3/bof3.h"

typedef void (*Boss05516Handler)(void);

void func_800C2780(void);
void func_800C27E8(void);

/* @source 0x800C2728
 * @behavior Overlay step dispatch on the state byte of the work record
 * published at scratchpad pointer cell 0x1F800044: seeds a local two-entry
 * handler array with 0x800C2780 then 0x800C27E8, and calls the entry selected
 * by the record's unsigned byte +2 without arguments. The original reads the
 * work-record pointer once, fills both frame slots ahead of the byte load and
 * does not bound the index, and it writes no state of its own: the 0x20-byte
 * frame only keeps $ra across the indirect call. The state byte is the same
 * one this overlay's step dispatcher selects this function with at run index 9
 * of the 0x800C35F8 handler run, and entry 0 (0x800C2780) advances it by one
 * after its setup work, so the pair is this overlay's step 0 and step 1 bodies.
 * No word of the payload holds this function's address, so it is an overlay
 * entry reached from outside the payload; the prologue and return path alone
 * place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2LocalPairBossBoss05516_800C2728(void) {
  Boss05516Handler handlers[2] = { func_800C2780, func_800C27E8 };

  handlers[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}

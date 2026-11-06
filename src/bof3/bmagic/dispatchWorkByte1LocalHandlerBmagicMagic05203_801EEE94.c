#include "bof3/bof3.h"

typedef void (*BmagicMagic05203Handler)(void);

/* The single handler this dispatcher seeds is 0x801EEEE0, the following
 * function of this overlay: it copies the three-word code pointer table at
 * 0x801EEC10 into its own frame and dispatches on work byte 2. */
void func_801EEEE0(void);

/* @source 0x801EEE94
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once,
 * seeds the handler 0x801EEEE0 into entry 0 of a two-entry local handler array
 * in its own 0x20-byte frame, and invokes the entry selected by the work
 * object's unsigned byte at offset 1, with no arguments. The original leaves
 * entry 1 of the local array unseeded and does not check the index, so the
 * declaration keeps two entries and only entry 0 is assigned. It reads that one
 * byte and no other state, writes no memory apart from its own frame, and
 * returns nothing. The 76-byte body is instruction-for-instruction identical to
 * dispatchByte1PairTable (battle/battle/03 at 0x801E72A8) apart from the seeded
 * handler address, and its `lui`/`addiu` pair for that address is the only
 * difference from the byte image of that sibling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1LocalHandlerBmagicMagic05203_801EEE94(void) {
  BmagicMagic05203Handler handlers[2];

  handlers[0] = func_801EEEE0;
  handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}

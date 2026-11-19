#include "bof3/bof3.h"

typedef void (*BmagicMagic04500Handler)(void);

/* Both handlers keep their address-anchored names: this payload only stores
 * their addresses, and the overlay loader is the sole caller of this step. */
void func_801EEC5C(void);
void func_801EEE58(void);

/* @source 0x801EEC04
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once,
 *           seeds 0x801EEC5C into entry 0 and 0x801EEE58 into entry 1 of a
 *           two-entry local handler array in its own 0x20-byte frame, and
 *           invokes the entry selected by the work object's unsigned byte at
 *           offset 1, with no arguments. The index is unchecked, as in the
 *           original, which reads no other state and writes nothing outside
 *           its own frame; that frame only keeps $ra across the indirect call.
 *           Comparing the payload spans showed the 88-byte body equal to
 *           dispatchWorkByte1LocalPairBmagicMagic00303_801EF2CC apart from the
 *           two %lo words that carry the handler addresses.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1LocalPairBmagicMagic04500_801EEC04(void) {
  BmagicMagic04500Handler handlers[2] = {func_801EEC5C, func_801EEE58};

  handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}

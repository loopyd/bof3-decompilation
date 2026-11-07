#include "bof3/bof3.h"

extern u8 D_8014933B;

/* Overlay drawing routine of this payload. It builds the record's quad
 * primitive (SetPolyFT4, SetShadeTex) for three consecutive selections and
 * projects it with RotTransPers4, so its two arguments are the selection
 * index and a signed byte mode. It stays address-anchored until it is lifted. */
void func_801F37B8(s32 arg0, s32 arg1);

/* @source 0x801F3788
 * @behavior Handler at word 3 of the overlay's state-byte-2 dispatch run based
 *           at 0x801F5024, so it has no jal caller: the dispatcher at
 *           0x801F35D8 reaches it. Writes the constant 0x01 to the shared
 *           overlay byte at 0x8014933B and then calls the overlay's quad
 *           drawing routine at 0x801F37B8 with the selection index 2 and the
 *           signed byte mode 0. The constant is materialised before the frame
 *           is saved and 0 is passed in the call's delay slot. Takes no
 *           arguments and returns nothing; its 0x18-byte frame only holds $ra
 *           at 0x10 across the call.
 *
 * Name evidence: the body's own two actions are the shared state byte write and
 * the draw call, which is the order the name records; the callee's body
 * evidences the drawing role (SetPolyFT4, SetShadeTex, RotTransPers4 over the
 * record published at the scratchpad cell 0x1F800044). The draw arguments are
 * kept in the metadata rather than in the name because the callee is not
 * lifted, so the selection index is not yet independently named for this
 * payload.
 *
 * Shape evidence: no other payload carries these 48 bytes - a scan of the 394
 * overlay binaries finds this range only here - and its two neighbours in the
 * dispatch run (0x801F3720, and the state-2 byte writer at 0x801F3990) differ
 * in body, so neither clone eligibility nor a shared shape applies.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSharedByte1ThenDrawWorld03Area14813_801F3788(void) {
  D_8014933B = 1;
  func_801F37B8(2, 0);
}

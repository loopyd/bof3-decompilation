#include "bof3/bof3.h"

void func_8015DF18(u16 arg0);

/* Shared published work-pointer cell. Declared plain, not volatile: the one
 * reload this body shows follows the published store to the record byte at +9,
 * which may alias the cell itself, so the compiler re-materialises the cell
 * address and reloads the pointer with no volatile annotation needed. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F30C0
 * @behavior Entry 0 of the byte-1 handler table based at 0x801F3E00, whose
 *           three words hold this address, 0x801F312C at 0x801F3E04 and
 *           0x801F31EC; the table is indexed with the work record state byte
 *           at +1 by this target exact dispatcher
 *           dispatchStateByte1Table3e00WorldWorld03Area13213_801F307C.
 *           Loads the published work record through the pointer cell
 *           0x1F800044 and copies its saved words at +0x34, +0x38 and +0x3C
 *           into the record words at +0x0C, +0x10 and +0x14, starting the
 *           interpolation word at +0x1C from that same +0x3C copy. It arms the
 *           countdown byte at +0x09 with 0x40, clears the halfword at +0x20 and
 *           stores the 0x01000000 step in the word at +0x24, the step entry 1
 *           at 0x801F312C adds to +0x1C and clamps against +0x14. It then reads
 *           the cell again, advances the state byte at +0x01 by one so the next
 *           frame dispatches entry 1, and passes the cue 0x207 to the resident
 *           cue dispatcher 0x8015DF18, whose call delay slot keeps that state
 *           byte store. Takes no arguments and returns nothing; its 0x18-byte
 *           frame holds $ra at 0x10 across the call. The roles of the record
 *           words at +0x0C, +0x10, +0x14 and +0x1C, of the halfword at +0x20 and
 *           of the saved words at +0x34, +0x38 and +0x3C stay described by
 *           offset.
 *
 * Name evidence: the only in-image reference to 0x801F30C0 is the word at
 * 0x801F3E00, entry 0 of the pointer run based there; there is no jal caller,
 * which is the dispatch shape rather than missing evidence. The seed, cue and
 * state-advance roles come from this body own literals and stores, and the
 * exact same-target entry 1 handler at 0x801F312C moves the fields this body
 * seeds.
 *
 * Shape evidence: the seed-words-then-store-step-then-advance-state-then-cue
 * shape, the statement order, the pointer-cell reload point and the plain
 * offset cast operand form follow the exact
 * clearWorkTableSeedWorkWords34To3CAnd0CTo14DispatchCue208AndAdvanceWorkByte1World03Area13413_801F4388,
 * the same seed body in the world03 area134 overlay that differs only in its
 * leading work-table clear call and in the cue it dispatches (0x208 there);
 * this function own cue 0x207, offsets and absent leading call come from its
 * original bytes. No byte-identical copy of these 0x6C bytes exists in this
 * tree: a scan of every payload under out/binaries for the 0x207 cue immediate
 * found 13 sites and only this target matches on the body, so the clone lever
 * is declined and this body is seeded from original bytes.
 *
 * Matching note: the three saved-word loads must sit immediately after the cell
 * load, before the countdown byte is armed. Written as plain statements they
 * are live single-definition pseudos that the scheduler sinks past the arm
 * store and the cell reload, which fills the load delay slot with a nop
 * instead (22/27, 108->108, first mismatch at +0x00 after a reordered
 * prologue). A single-execution scope around that load group puts a loop note
 * between it and the arm store, whose pending-read dependencies pin the loads
 * there; that measured 27/27 at 108->108 bytes with no remaining difference.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkWords34To3CAnd0CTo14DispatchCue207AndAdvanceWorkByte1World03Area13213_801F30C0(
    void) {
  u8* work;
  u8* state;
  s32 at34;
  s32 at38;
  s32 at3c;

  work = D_1F800044;
  do {
    at34 = *(s32*)(work + 0x34);
    at38 = *(s32*)(work + 0x38);
    at3c = *(s32*)(work + 0x3C);
  } while (0);
  work[9] = 0x40;
  state = D_1F800044;
  *(u16*)(work + 0x20) = 0;
  *(s32*)(work + 0x24) = 0x1000000;
  *(s32*)(work + 0x0C) = at34;
  *(s32*)(work + 0x10) = at38;
  *(s32*)(work + 0x14) = at3c;
  *(s32*)(work + 0x1C) = at3c;
  state[1]++;
  func_8015DF18(0x207);
}

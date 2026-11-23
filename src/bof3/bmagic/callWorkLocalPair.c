#include "bof3/bof3.h"

typedef void (*BmagicMagic06903Handler)(void);

/* Entry 0 of the local pair is this overlay's own leading code blob at
 * 0x801EEC5C, still unlifted, so it keeps its address-anchored name. Entry 1 is
 * the already-exact lift of 0x801EEE30 in this target. No jal and no pointer
 * word inside this payload names 0x801EEC04, so the overlay loader is the only
 * caller of this body and both handler roles stay structural.
 */
void func_801EEC5C(void);
void forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic06903_801EEE30(
    void);

/* @source 0x801EEC04
 * @behavior Overlay step dispatcher: reads the scratchpad work-object pointer
 *           cell at 0x1F800044 once, seeds 0x801EEC5C into entry 0 and
 *           0x801EEE30 into entry 1 of a two-entry local handler array in its
 *           own 0x20-byte frame, and invokes the entry selected by the work
 *           object's unsigned byte at offset 1, with no arguments. The index is
 *           unchecked, as in the original, which reads no other state and
 *           writes nothing outside its own frame; that frame only keeps $ra
 *           across the indirect call. Comparing the payload spans showed the
 *           88-byte body equal to
 *           dispatchWorkByte1LocalPairBmagicMagic04500_801EEC04 apart from the
 *           one %lo word that carries entry 1's handler address (0x801EEE30
 *           here, 0x801EEE58 there); entry 0's handler address is the same word
 *           in both, and every other word - frame, register allocation, the
 *           byte-1 selector load and the stack-table call - is equal.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callWorkLocalPair(void) {
  BmagicMagic06903Handler handlers[2] = {
      func_801EEC5C,
      forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic06903_801EEE30};

  handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}

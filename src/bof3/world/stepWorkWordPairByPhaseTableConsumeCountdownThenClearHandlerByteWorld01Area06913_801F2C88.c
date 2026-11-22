#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */
extern s8 D_801F3DF8[]; /* @source 0x801F3DF8 @kind table */

/* @source 0x801F2C88
 * @behavior Per-frame step of the work record published through the
 *           scratchpad pointer cell 0x1F800044, gated on its phase byte at
 *           0x0A. While that byte is set it adds the signed entry of this
 *           payload's 16-entry phase table at 0x801F3DF8 selected by the low
 *           nibble of the phase byte, scaled by 0x800, to the record word at
 *           0x34, subtracts the entry selected by a fresh read of the same
 *           nibble from the record word at 0x38, decrements the phase byte and
 *           consumes two units of the halfword at 0x122 of the record
 *           published through the global pointer cell 0x80146250. Once the
 *           phase byte reads zero it instead clears the handler byte at 0x04,
 *           which is the index of this overlay's two-entry handler table at
 *           0x801F3DF0, so the next dispatch reaches the armer at 0x801F2C48
 *           that seeds the phase byte with 0x40 and arms entry 1 again. The
 *           overlay reaches this body as entry 1 of that table, so the
 *           parameter roles stay structural: no arguments and no return
 *           value.
 *
 * Shape evidence: a word comparison of this 36-word span against every
 *   extracted binary under out/binaries found no byte-identical copy, so the
 *   clone lever was declined. The nearest relative is the world01 area068 step
 *   at 0x801F3CB0, whose span differs in exactly three words: the two %lo
 *   halves of the load that reads its own phase table at 0x801F4DEC, and the
 *   absolute j word, which shifts by 0x1028, the address delta between the two
 *   spans. Every other word, the shared pointer cell 0x80146250 and the
 *   halfword offset 0x122 included, is equal, so that body supplied the
 *   statement order, declaration style and offset spelling while every literal
 *   and binding here comes from this span's own bytes. The same shape is
 *   lifted in world01 area039 at the same address 0x801F2C88, which differs
 *   only in its phase table, its shared cell and its halfword offset.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The original lets its test load feed the first table index and re-reads the
 * phase byte for the second index and for the decrement, so those three reads
 * are volatile; the record words and the shared record stay plain because the
 * original lets the scheduler interleave those accesses.
 */
void stepWorkWordPairByPhaseTableConsumeCountdownThenClearHandlerByteWorld01Area06913_801F2C88(void) {
  volatile u8* work;
  u8* shared;
  s32 step;
  u8 phase;

  work = D_1F800044;
  phase = work[0x0a];
  if (phase != 0) {
    *(s32*)(work + 0x34) += D_801F3DF8[phase & 0x0f] << 11;
    step = D_801F3DF8[work[0x0a] & 0x0f] << 11;
    work[0x0a] = work[0x0a] - 1;
    shared = D_80146250;
    *(s32*)(work + 0x38) -= step;
    *(u16*)(shared + 0x122) -= 2;
  } else {
    work[4] = 0;
  }
}

#include "bof3/bof3.h"

/* Fixed-address state cell named by this target's own map: the scratchpad
 * work-pointer cell at 0x1F800044. The group that seeds the position pair reads
 * it once into a local; the later groups read it inline, and the two inline
 * reads of the countdown group fold into the single cell load the original bytes
 * show there. */
extern u8* scenarioScena0700Work; /* @source 0x1F800044 @kind unknown */

/* First word of the shared coordinate pair in main RAM that this handler
 * copies into the work object's position pair.
 * @source 0x80145EC4 @kind data
 */
extern s32 D_80145EC4;

/* Second word of that shared coordinate pair.
 * @source 0x80145EC8 @kind data
 */
extern s32 D_80145EC8;

/* Shared distance helper in the main executable, outside this overlay's
 * payload; it takes the two coordinate words and answers the distance between
 * them.
 * @source 0x8015477C
 */
extern s32 func_8015477C(s32 x, s32 y);

/* Setup pass at 0x801FA01C, an `asm` row of this target: its 0x370 bytes clear
 * the shared 0x800E595C..0x800E5964 state, zero the 0x800E81D8 word, store
 * 0x1000 in 0x800E81DC and 0x800E81E0, walk the 0x20 slots at 0x800E6FF4 and
 * reach func_801782FC, func_801783C8, func_801786E8 and func_80178818. It
 * reads no argument: every use of $a0 in its body is a write.
 * @source 0x801FA01C
 */
extern void func_801FA01C(void);

/* @source 0x801F7D6C
 * @behavior One-shot overlay handler, word 0 of the ten-word in-image
 * code-pointer run based at 0x801FDDC8 (the word at 0x801FDDC8), which
 * dispatchWorkByte2HandlerScenarioScena0700_801F7D28 indexes with the
 * scratchpad work object's unsigned byte at offset 0x02, so it runs while that
 * byte reads 0. The address is also word 2 of the twelve-word run based at
 * 0x801FDDC0 that dispatchWorkByte2HandlerScenarioScena0700_801F7B9C indexes;
 * no in-image `jal` reaches it and those two run words are the only image words
 * naming it. It loads the work-object pointer cell at 0x1F800044 before it
 * creates its frame, seeds that object's word at 0x34 from the shared
 * coordinate word D_80145EC4 and its word at 0x38 from D_80145EC8, and then
 * stores into the object's word at 0x3C the distance the shared main-executable
 * helper func_8015477C computes from that pair, shifted left by 16. That call
 * takes a read of the 0x34 word just stored as its first argument and the
 * D_80145EC8 value still in a register as its second, and the 0x38 store lands
 * in its delay slot, so the first call carries both the seeded coordinate and
 * the second coordinate. The 0x3C store then re-reads the pointer cell, because
 * the store follows the helper call in the original bytes. It next calls the
 * overlay's setup pass at 0x801FA01C with no arguments and discards the result.
 * Finally it reads the pointer cell a third time, arms the object's countdown
 * word at 0x0C with 0x78 and increments the object's byte at 0x02, the handler
 * index that advances this run, so the next frame runs word 1 at 0x801F7DDC,
 * whose body counts that word down and calls func_80196070 when it reaches
 * zero. The 0x78 is materialised behind the countdown store and the incremented
 * byte is written after $ra is reloaded. Takes no arguments and returns
 * nothing.
 *
 * Representation notes. Three measurements fixed this body. The 0x28-byte frame
 * is 0x10 over the 0x18 a call-making body of these locals reserves, and the
 * original has no access to that space, so it carries one unused 16-byte local:
 * a used scratch region kept its own stores and an unused one matched. The
 * helper call's first argument is a genuine reload of the word just stored
 * (`lw $a0,0x34($v1)`), so that read is qualified volatile, without which the
 * build forwards the stored value and drops the load. The countdown group reads
 * the pointer cell inline at both of its uses, and the two reads fold into one
 * cell load placed after the setup call, which is where the original builds the
 * third `lui`/`lw` pair; a separate pointer local there kept the cell load
 * before the call and cost two callee-saved slots.
 * @status exact
 * @match 100.00
 * @residual none
 */
void statePairDistanceArm(void) {
  u8* work;
  s32 pairA;
  s32 pairB;
  /* Frame reservation only: the original owns 0x10 bytes of stack that no
   * instruction touches, so no use is made of this region. */
  u32 pad[4];

  work = scenarioScena0700Work;
  *(s32*)&work[0x34] = D_80145EC4;
  pairB = D_80145EC8;
  pairA = *(volatile s32*)&work[0x34];
  *(s32*)&work[0x38] = pairB;
  *(s32*)&scenarioScena0700Work[0x3C] = func_8015477C(pairA, pairB) << 16;
  func_801FA01C();
  *(s32*)&scenarioScena0700Work[0x0C] = 0x78;
  scenarioScena0700Work[2]++;
}

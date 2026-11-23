#include "bof3/bof3.h"

/* Fixed-address state cell named by this target's own map: the scratchpad
 * work-pointer cell at 0x1F800044. Each source-level read keeps its own cell
 * load, because the handler-advance arm's pointer read follows stores through
 * the pointer an earlier read returned. */
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

/* Block setup pass at 0x801F90F4, an `asm` row of this target: it takes the
 * 0x800E4C24 block's own base, seeds the three words that base reaches at
 * +0x514 through +0x51C, and walks the block's 0x20 0x80-byte-stride slots; the
 * word at +0x510 it reads before each slot is the bound this handler writes as
 * the halfword 0x800E5134. It answers a leftover register, so this handler
 * discards the call's result.
 * @source 0x801F90F4
 */
extern void func_801F90F4(u8* block);

/* @source 0x801F7BE0
 * @behavior One-shot overlay handler, word 0 of the twelve-word in-image
 * code-pointer run based at 0x801FDDC0, which
 * dispatchWorkByte2HandlerScenarioScena0700_801F7B9C indexes with the scratchpad
 * work object's unsigned byte at offset 0x02, so it runs while that byte reads
 * 0; that run word is the only image word naming the address and no in-image
 * `jal` reaches it. It loads the work-object pointer cell at 0x1F800044 before
 * it creates its frame, seeds that object's word at 0x34 from the shared
 * coordinate word D_80145EC4 and its word at 0x38 from D_80145EC8, and then
 * stores into the object's word at 0x3C the distance the shared main-executable
 * helper func_8015477C computes from that pair, shifted left by 16, through a
 * second read of the pointer cell. It publishes that same pair and distance
 * into the shared main-RAM block at 0x800E4C24: word 0x800E4C24 takes the
 * object's 0x34 word, 0x800E4C28 its 0x38 word and 0x800E4C2C its re-read 0x3C
 * distance plus 0x2000000, while the halfword at 0x800E5134 - offset 0x510 of
 * that block - takes 0x800, and the block's base goes into this overlay's
 * pointer cell at 0x801FDFC0. The address 0x800E4C24 is materialised once for
 * the first copy and that same register is both stored into the cell and used
 * as the base of the copy, and it is materialised again for the call that
 * follows: the overlay's block setup pass at 0x801F90F4 takes the block's base
 * and rewrites the fields around offset 0x514. Through a third read of the
 * pointer cell the handler then arms the object's countdown word at 0x0C with
 * 0x28 and increments the object's byte at 0x02, the handler index that
 * advances this run. Takes no arguments and returns nothing; its 0x28-byte
 * frame keeps only $ra at 0x20, and that frame is 0x10 bytes wider than the
 * space these locals need, so its only local below $ra is untouched stack.
 *
 * Representation notes. Seeded from this target's exact siblings the
 * 0x801F7D6C and 0x801F7A30 distance-pair handlers for the position-pair
 * prologue and from the 0x801F7638 handler for the fixed-address publish group
 * and the pointer-cell store. The clone lever was declined: a scan of the 398
 * payloads under out/binaries for this 0xBC-byte span matched the span itself
 * only, so no byte-identical twin exists anywhere. Probes before this body,
 * every one of them 188 bytes unless noted: the publish group in the order
 * 0x800E4C24 / 0x801FDFC0 / 0x800E4C28 / 0x800E5134 / 0x800E4C2C measured
 * 37/48 at 192 bytes, and the same group with 0x800E4C28 ahead of the cell
 * measured 36/47; a call-result local for the 0x3C store, a chained assignment
 * of the 0x38 word, and the 0x34 re-read written first each measured the same
 * 36/47. Reading the 0x800E4C2C addend into its own local after the cell store
 * measured 46/47 and is the shape kept here, so the publish group's reads and
 * the halfword constant fall where the original shows them. Further pre-call
 * orders - the 0x34 re-read written first, that re-read inlined in the call's
 * first argument, a call-result local, and the 0x38 store written before the
 * re-read - all measured 46/47 with the same first difference.
 * The unscoped readback measured 46/47 at 188 bytes: the second scheduler
 * moved D_80145EC8 ahead of the 0x34 readback. A scope around the first
 * coordinate's store and readback measured 45/48 at 192 bytes, while a scope
 * around only the readback kept the original load order at 47/47, 188 bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateDistanceBlockArm(
    void) {
  u8* work;
  u8* saved;
  u8* advance;
  s32 pairA;
  s32 pairB;
  s32 dist3c;
  /* Frame reservation only: the original owns 0x10 bytes of stack that no
   * instruction touches, so no use is made of this region. */
  u32 pad[4];

  work = scenarioScena0700Work;
  *(s32*)&work[0x34] = D_80145EC4;
  do {
    pairA = *(volatile s32*)&work[0x34];
  } while (0);
  pairB = D_80145EC8;
  *(s32*)&work[0x38] = pairB;
  *(s32*)&scenarioScena0700Work[0x3C] = func_8015477C(pairA, pairB) << 16;
  saved = scenarioScena0700Work;
  PSX_REF(s32, 0x800E4C24u) = *(s32*)(saved + 0x34);
  PSX_REF(s32, 0x800E4C28u) = *(s32*)(saved + 0x38);
  PSX_REF(u8*, 0x801FDFC0u) = PSX_PTR(u8, 0x800E4C24u);
  dist3c = *(s32*)(saved + 0x3C);
  PSX_REF(u16, 0x800E5134u) = 0x800;
  PSX_REF(s32, 0x800E4C2Cu) = dist3c + 0x2000000;
  func_801F90F4(PSX_PTR(u8, 0x800E4C24u));
  advance = scenarioScena0700Work;
  *(s32*)(advance + 0x0C) = 0x28;
  advance[2]++;
}

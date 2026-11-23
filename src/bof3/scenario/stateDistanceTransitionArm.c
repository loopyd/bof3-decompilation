#include "bof3/bof3.h"

/* Fixed-address state cell named by this target's own map: the scratchpad
 * work-pointer cell at 0x1F800044. Each source-level read of it keeps its own
 * named-cell load, because a store through the pointer it returned could write
 * that very cell, so the handler-advance arm's pointer read is a separate
 * `lui`/`lw` pair from the saved pointer the two field stores run through. */
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

/* Table-clearing helper at 0x801F8358: it zeroes the lead byte of each of the
 * eight 0x1C-byte-stride records at 0x800E4800 and then the two flag bytes at
 * 0x801FDFC4 and 0x801FDFC8, and it takes no argument. */
void stateTableFlagBytes(void);

/* @source 0x801F7A30
 * @behavior One-shot overlay handler, word 0 of the three-word in-image
 * code-pointer run labelled T_801FDDB4 at 0x801FDDB4, which
 * dispatchWorkByte2HandlerScenarioScena0700_801F79EC indexes with the
 * scratchpad work object's unsigned byte at offset 0x02, so it runs while that
 * byte reads 0. No in-image `jal` reaches the address and that run word is the
 * only image word naming it. It loads the work-object pointer cell at
 * 0x1F800044 before it creates its frame, seeds that object's word at 0x34 from
 * the shared coordinate word D_80145EC4 and its word at 0x38 from D_80145EC8,
 * and then stores into the object's word at 0x3C the distance the shared
 * main-executable helper func_8015477C computes from that pair, shifted left by
 * 16. That call takes a read of the 0x34 word just stored as its first argument
 * and the D_80145EC8 value still in a register as its second, and the 0x38
 * store lands in its delay slot, so the first call carries both the seeded
 * coordinate and the second coordinate. The 0x3C store comes after the helper
 * call, through a second read of the pointer cell. It next calls the overlay's
 * table-clearing helper at 0x801F8358 with no arguments and discards the
 * result, and then, through a third read of the pointer cell, writes 1 to the
 * work object's transition byte at offset 0x06, arms the object's countdown
 * word at 0x0C with 0x28 and increments the object's byte at 0x02, the handler
 * index that advances this run, through a fourth read of that cell. The 0x28 is
 * materialised after the fourth read and the incremented byte is written after
 * $ra is reloaded. Takes no arguments and returns nothing, and its 0x28-byte
 * frame keeps only $ra at 0x20.
 *
 * Representation notes. Seeded from the already-exact same-target sibling
 * statePairDistanceArm
 * (0x801F7D6C), whose first 20 instructions equal this span's word for word and
 * which supplied the statement order, operand forms, offset spelling and
 * declaration style; this span's own bytes supplied the setup callee
 * (0x801F8358 here, 0x801FA01C there), the added transition-byte store, the
 * 0x28 countdown literal and the second cell read the advance arm needs. The
 * clone lever was declined: a scan of every shipped payload under out/binaries
 * for this span matched none, so no byte-identical twin exists. The 0x28-byte
 * frame is 0x10 over the 0x18 a call-making body of these locals reserves, and
 * the original has no access to that space, so it carries one unused 16-byte
 * local. The three pointer locals are load-bearing and two of them are not
 * merged: the pre-call read, the saved post-call pointer the transition-byte
 * and countdown stores run through, and the advance arm's own cell read, which
 * the original schedules between that byte store and the countdown store.
 * Probes before the first gate: one post-call pointer plus an inline advance
 * read measured 28/32 (128 -> 132 bytes), the advance read left after the
 * countdown store; adding the advance pointer while the pre-call read shared
 * the saved pointer measured 20/32 (128 -> 128 bytes), because that build put
 * the saved pointer in $a2 and shifted the whole allocation. Separate pre-call
 * and post-call pointers alongside the advance pointer measured 32/32 at 128
 * bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateDistanceTransitionArm(
    void) {
  u8* work;
  u8* saved;
  u8* advance;
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
  stateTableFlagBytes();
  saved = scenarioScena0700Work;
  saved[6] = 1;
  advance = scenarioScena0700Work;
  *(s32*)(saved + 0xC) = 0x28;
  advance[2]++;
}

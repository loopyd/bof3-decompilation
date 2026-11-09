#include "bof3/bof3.h"

/* PsyQ libc random source at 0x8017E3D4, the address this body's jal word
 * encodes. The answer is discarded: the call's $v0 is overwritten by the first
 * halfword load, so the declaration carries the library's return type with no
 * use in this function. */
extern int rand(void);

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays
 * and bound at 0x1F800044 by this target's map. The original re-materialises
 * the cell with its own lui/lw pair for each of the three reads this body makes
 * inside one basic block (offsets 0x19BC, 0x19D8 and 0x19F8), even across the
 * halfword and byte stores it issues through the pointer it loaded, so the cell
 * is declared volatile: every dereference of g_battle_work is one load of the
 * cell. The non-volatile spelling merges those three reads into one live base
 * and measures 124 bytes against the original 140. The same qualifier carries
 * the exact lifts of this family, for example battle15's g_battle_work and
 * magic106/03's D_801F05F8, both of whose bodies re-read their cell per access. */
extern u8 *volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F05AC
 * @behavior Entry 1 of the three-entry handler table at 0x801F0A2C that this
 *           overlay's work-byte-2 dispatcher
 *           dispatchWorkByte2HandlerThenDrawWhenActiveBmagicMagic04403_801F0478
 *           indexes with the scratchpad work object's byte 0x2, so no jal in
 *           this payload reaches this address; the table words
 *           0x801F04F0/0x801F05AC/0x801F0638 are the three state blocks that
 *           follow that dispatcher. It calls rand() and discards the answer,
 *           then reads the work pointer cell and decrements the work object's
 *           unsigned halfword 0x2E by one (lhu / addiu -1 / sh). It re-reads
 *           the cell, adds four modulo 256 to work byte 9 (lbu / addiu 4 / sb)
 *           and, through that same loaded base, decrements the unsigned
 *           halfword 0x30 by one, so the second halfword store shares the base
 *           of the byte-9 update and stays ordered after the byte store. It
 *           then re-reads the cell once more and, only while work byte 9 is
 *           exactly 0x10, increments work byte 2 modulo 256, which is the
 *           handler index the dispatcher reads and therefore advances this
 *           overlay from this state block to the run's next one; every other
 *           byte-9 value branches straight to the epilogue. The four-step
 *           advance lands only on multiples of four, so 0x10 is the third frame
 *           of this step. Takes no arguments, returns nothing, and the
 *           0x18-byte frame keeps only $ra at 0x10. The roles of work byte 9 and
 *           of the halfword pair remain unestablished, so the name stays
 *           offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void consumeRandomDecayWorkHalfwords2EAnd30AdvanceByte9By4AdvanceWorkByte2At10BmagicMagic04403_801F05AC(void) {
  u8 *work;
  u8 *state;
  u16 *halves;

  rand();
  work = g_battle_work;
  halves = (u16 *)work;
  halves[0x17] -= 1;
  work = g_battle_work;
  work[9] += 4;
  halves = (u16 *)work;
  halves[0x18] -= 1;
  state = g_battle_work;
  if (state[9] == 0x10) {
    state[2]++;
  }
}

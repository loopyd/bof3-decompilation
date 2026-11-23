#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays; this target's map binds it at 0x1F800044. The original reads the
 * cell twice: one load supplies the whole first group (lui $v1,0x1F80 / lw
 * $v1,0x44($v1), load-delay nop) and the trailing test re-reads the symbol
 * after the byte-width store (lui $a1,0x1F80 / lw $a1,0x44($a1)). A word
 * view retains the first base for the word stores; byte accesses use the
 * named cell so the trailing test reloads it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF0A4
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 and
 *           performs the work object step: it adds work byte 0xA into work
 *           word 0x20 (lbu / lw / addu) and stores the sum back to word 0x20,
 *           decrements work byte 0x9 by two at byte width (lbu / addiu
 *           $v0,$v0,-2 / sb), and adds that same sum onto work word 0x14
 *           (lw / addu / sw), so word 0x14 takes the updated word 0x20 and not
 *           raw byte 0xA. It then re-reads the cell (a second lui / lw pair)
 *           and tests the re-read work byte 0x9 against 16 (lbu / addiu
 *           $v0,$zero,0x10 / bne, empty delay slot); only when that byte
 *           equals 16 it increments work byte 0x1 by one at byte width (lbu /
 *           addiu $v0,$v0,1 / sb) through the reloaded base. Takes no
 *           arguments, returns nothing, keeps no frame and touches no other
 *           state.
 *
 *           The payload word at 0x801EFCD8 holds this address: it is the
 *           second entry of the three-pointer run at 0x801EFCD4 (0x801EEFF4,
 *           0x801EF0A4, 0x801EF108) that closes this payload, whose last
 *           declared asm row still reads those three cells as code. No jal in
 *           this payload reaches it, which is the dispatch shape rather than a
 *           missing caller, so the parameter roles stay structural and no
 *           prototype is owed. A scan of every extracted payload under
 *           out/binaries found no byte-identical copy of this 0x64-byte span;
 *           the span carries no jal word and no external data word, so no
 *           masked re-scan is owed.
 * @status partial
 * @match 92.00
 * @residual The byte-9 load precedes the word-20 load, and its decrement
 *           precedes the step addition. The original has those two pairs
 *           in the opposite order. All other instructions, registers,
 *           memory events and the 0x64-byte size match.
 */
void workIntoTwoSixteen(void) {
  s32 *words = (s32 *)g_battle_work;
  s32 step;

  step = words[8] + g_battle_work[0xA];
  g_battle_work[9] -= 2;
  words[8] = step;
  words[5] += step;
  if (g_battle_work[9] == 0x10) {
    g_battle_work[1]++;
  }
}

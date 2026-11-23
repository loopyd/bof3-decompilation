#include "bof3/bof3.h"

/* Random helper of the main executable. Splat binds it through the shared
 * PsyQ map row rand = 0x8017E3D4, outside this overlay's image, so no
 * target-map row is needed. */
extern int rand(void);

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map and by every exact sibling of this handler in the same overlay family.
 * It stays a named cell rather than a constant-address memory accessor because
 * the original re-materialises the base for each store group
 * (lui $v1,0x1F80 / lw $v1,68($v1)): three separate lui/lw pairs appear in the
 * body, since a store through the loaded pointer may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF1D4
 * @behavior Step 0 of the work-byte-2 step table based at 0x801EFAA8, the
 *           word that handler 0x801EF158 indexes without a bounds test. Seeds
 *           the work object reached through the scratchpad pointer cell at
 *           0x1F800044 from one rand() call: work word 0xC takes the low six
 *           bits of the result and work word 0x10 takes that value plus 0x1F,
 *           both written through one cached object base. It then re-reads the
 *           pointer cell, clears work byte 9, re-reads the cell again, sets
 *           work byte 0xA to 0x3C and increments work byte 2 modulo 256, which
 *           selects the next step of the table. Takes no arguments, returns
 *           nothing and keeps a 0x18-byte frame that saves $ra. Word and byte
 *           roles remain offset-anchored; byte 0xA is the countdown that step
 *           0x801EF288 decrements and byte 9 the counter that step 0x801EF238
 *           raises to 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateWordsRandClear(void) {
  u32 *words;
  u32 seed;

  seed = rand() & 0x3F;
  words = (u32 *)g_battle_work;
  words[3] = seed;
  words[4] = seed + 0x1F;
  ((u8 *)words)[9] = 0;
  g_battle_work[0xA] = 0x3C;
  g_battle_work[2]++;
}

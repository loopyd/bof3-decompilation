#include "bof3/bof3.h"

/* Each store through the loaded pointer can alias this scratchpad cell, so
 * subsequent accesses read the cell again, as the original instructions do. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F02FC
 * @behavior Adds two modulo 256 to work byte 0xA, then reloads the scratchpad
 *           work pointer and adds four modulo 256 to work byte 9. When that
 *           stored byte equals 0x10, reloads the pointer and increments work
 *           byte 2. The pointer cell is at 0x1F800044. This leaf takes no
 *           arguments and returns nothing; its address is stored at 0x801F08F8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add2ToWorkByteAThenAdd4ToWorkByte9AndAdvanceWorkByte2At16BmagicMagic12403_801F02FC(void) {
  u8 *work;
  u8 counter;

  work = g_battle_work;
  work[0xA] += 2;
  counter = g_battle_work[9] + 4;
  g_battle_work[9] = counter;
  if (counter == 0x10) {
    g_battle_work[2]++;
  }
}

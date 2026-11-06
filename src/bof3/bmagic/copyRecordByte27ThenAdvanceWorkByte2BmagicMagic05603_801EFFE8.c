#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * handlers. Declared as the same volatile u8 pointer the exact bmagic siblings
 * use (seedMotionTintAndChannelFieldsThenAdvanceWorkByte2BmagicMagic00800_
 * 801F05FC copies the same record byte through it): the original reads the cell
 * again before the increment group, and the named symbol lets the assembler fold
 * the constant address into the load (lui $v1,0x1F80 / lw $v1,68($v1)). */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work pointer in main RAM, outside this overlay's image. The
 * bmagic siblings read the same record through this pointer; this entry copies
 * its 0x27 byte. Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EFFE8
 * @behavior Overlay step handler: copies byte 0x27 (the elevation byte the
 *           sibling seedWorkObjectFromBattleRecord uses the same way) of the
 *           shared battle work record at 0x801EB4E0 into byte 0x27 of the work
 *           object published at scratchpad pointer cell 0x1F800044, then
 *           advances work byte 2 with a byte-width read-modify-write. Takes no
 *           arguments and returns nothing. The original reads the scratchpad
 *           pointer cell once per store group.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyRecordByte27ThenAdvanceWorkByte2BmagicMagic05603_801EFFE8(void) {
  u8 *work;

  work = g_battle_work;
  work[0x27] = D_801EB4E0[0x27];
  work = g_battle_work;
  work[2]++;
}

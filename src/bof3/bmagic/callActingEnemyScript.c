#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through the encoded symbol name and this target's generated
 * undefined_syms_auto.txt, so no target-map row is needed. The exact sibling
 * seedWorkWordsAndCopyRecordByte27ClearByte9SetByteATo16ThenAdvanceWorkByte2
 * BmagicMagic00100_801EFBEC copies its byte 0x27 the same way. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */

/* Scratchpad work-object pointer cell. Declared as a named fixed address so the
 * assembler folds the constant into each access (lui $vN,0x1F80 / lw $vN,68($vN));
 * the original re-materialises the cell before every store group because a store
 * through the pointer it supplied may alias the cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Acting battler byte of the main executable; the shared helper below reads it as
 * the enemy work record index. */
extern u8 D_80146374; /* @source 0x80146374 @kind data */

/* Shared battle helper of the main executable, outside this overlay's image: it
 * publishes the 0x118-byte enemy work record at 0x801EB2E8 indexed by its first
 * argument as the current enemy work, runs the enemy script byte helper
 * func_801E2314 for the second argument, then restores the previously published
 * record. */
extern void statePublishEnemyWork(u8 index, u8 scriptByte); /* @source 0x801E257C @kind function */

/* @source 0x801EF0E8
 * @behavior Overlay work handler, word 3 of the leading 24-word code pointer run
 *           at 0x801EEC04, stored at 0x801EEC10. Runs the shared enemy script
 *           helper statePublishEnemyWork with the acting battler byte 0x80146374 as the
 *           enemy work index and 0xA as the script byte selector, then copies
 *           byte 0x27 of the shared battle work record read through 0x801EB4E0
 *           into byte 0x27 of the scratchpad work object read through the pointer
 *           cell 0x1F800044. It clears work byte 9 and work byte 0xA to zero and
 *           finally advances work byte 2, the handler index, with a byte-width
 *           read-modify-write. Each later store group reads the pointer cell
 *           through its own local because a store through the loaded pointer may
 *           alias the cell, and the record pointer load precedes the first cell
 *           load. The 0x18-byte frame only keeps $ra at 0x10 across the helper
 *           call. Takes no arguments, returns nothing and reads or writes no
 *           other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callActingEnemyScript(void) {
  u8 *source;
  u8 *work;
  u8 *clearByte9;
  u8 *clearByteA;
  u8 *step;

  statePublishEnemyWork(D_80146374, 0xA);
  source = D_801EB4E0;
  work = D_1F800044;
  work[0x27] = source[0x27];
  clearByte9 = D_1F800044;
  clearByte9[9] = 0;
  clearByteA = D_1F800044;
  clearByteA[0xA] = 0;
  step = D_1F800044;
  step[2]++;
}

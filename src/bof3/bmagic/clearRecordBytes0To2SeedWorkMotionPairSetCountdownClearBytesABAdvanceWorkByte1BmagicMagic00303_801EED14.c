#include "base/types.h"

/* Overlay work-record table at 0x801F0B38: 32 slots of 0x78 bytes, the same
 * 0x78-stride record shape the family's reserveFirstFreeRecord bodies walk.
 * func_801EEC28 indexes this base by record offset and tests byte 0 bit 0
 * before dispatching, so the three bytes this body clears are the record's
 * leading state bytes. The base is bound by this target's map row. */
extern u8 magic00303Records[]; /* @source 0x801F0B38 @kind data */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row; re-read before each later access group because a store through the
 * loaded pointer may alias the cell. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EED14
 * @behavior Clears the three leading bytes 0, 1 and 2 of all 32 work records at
 * 0x801F0B38, walking them with a byte offset stepped by the record stride
 * 0x78. It then seeds the scratchpad work object read through pointer cell
 * 0x1F800044: the motion pair halfwords at 0x2E and 0x30 become 0xA0 and 0x70,
 * byte 0xB, byte 0xA and countdown byte 9 become 0, 0 and 0x1E, and state byte
 * 1 is advanced by one. It re-reads the pointer cell before the byte 9 seed,
 * before the byte 0xA clear and before the byte 1 increment, which is the
 * original's four cell loads. Takes no arguments, returns nothing, calls
 * nothing and touches no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordBytes0To2SeedWorkMotionPairSetCountdownClearBytesABAdvanceWorkByte1BmagicMagic00303_801EED14(
    void) {
  u32 i;
  u32 offset;
  u8* work;
  u8* clear9;
  u8* clearA;
  u8* step;

  for (i = 0, offset = 0; i < 0x20; offset += 0x78, i++) {
    magic00303Records[offset] = 0;
    magic00303Records[offset + 1] = 0;
    magic00303Records[offset + 2] = 0;
  }
  work = g_battle_work;
  *(u16*)(work + 0x2E) = 0xA0;
  *(u16*)(work + 0x30) = 0x70;
  work[0xB] = 0;
  clear9 = g_battle_work;
  clear9[9] = 0x1E;
  clearA = g_battle_work;
  clearA[0xA] = 0;
  step = g_battle_work;
  step[1]++;
}

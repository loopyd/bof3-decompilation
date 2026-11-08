#include "bof3/bof3.h"

typedef struct {
  u8 unk_00[0x3E];
  u16 unk_3E;
} WorkRecord801F4150;

extern u8 D_80144ED0; /* @source 0x80144ED0 @kind unknown */
extern u8 D_801F5198[];
extern u8 D_801F51B0[];
extern u8 D_801F51B1[];
extern u16 D_801F51B4[];
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

void setWorkByte9To9AndAdvanceWorkByte2WhenIndexedFlagClearThenDrawFlagIndexedScaledFrameWorld03Area11813_801F4278(
    void);
void setWorkByte9To9AndAdvanceWorkByte2WhenIndexedFlagSetThenDrawWorld03Area11813_801F43FC(
    void);
s16 func_8015477C(s32 arg0, s32 arg1);
s32 func_8015B5D4(u32 arg0, s32 arg1);

/* @source 0x801F4150
 * @behavior Byte-2 state handler that the last byte-2 dispatch table of this
 *           overlay (0x801F519C, indexed by the dispatcher at 0x801F410C)
 *           lists at entry 0. Seeds the work record published at the
 *           scratchpad pointer cell 0x1F800044 from its target halfword at
 *           offset 0x36: stores the low byte of that halfword into the cursor
 *           byte at offset 0x0B, then writes the record word at 0x34 as the
 *           byte two cursor steps into table 0x801F51B0 shifted left 16 and
 *           the record word at 0x38 the same way from table 0x801F51B1, so the
 *           pair is the cell coordinate of the indexed slot. Calls the shared
 *           distance helper func_8015477C with that coordinate pair and stores
 *           minus half of the returned signed distance less the
 *           cursor-indexed halfword of table 0x801F51B4 into the record
 *           halfword at 0x3E, then advances the cursor byte at 0x0B. Probes
 *           the flag number the advanced cursor picks from table 0x801F5198 in
 *           the flag bank at 0x80144ED0 through the shared flag tester
 *           func_8015B5D4; when that flag is set it increments the record state
 *           byte at 0x02 and calls its sibling handler at 0x801F4278, otherwise
 *           it stores 3 into that state byte and calls the sibling handler at
 *           0x801F43FC. Takes no arguments and returns nothing; the record
 *           pointer is loaded before the frame is created and the 0x18-byte
 *           frame keeps only $ra at 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedRecordCursorCoordinatePairFromSlotTablesAndHalfDistanceThenAdvanceCursorAndDispatchFlagStateWorld03Area11813_801F4150(
    void) {
  u8* work;
  u8* cell;
  u8* target;
  u8* state;
  u8* probe;
  s32 y;
  s16 distance;

  work = D_1F800044;
  work[0xB] = (u8) * (u16*)(work + 0x36);

  cell = D_1F800044;
  *(u32*)(cell + 0x34) = (u32)D_801F51B0[cell[0xB] * 2] << 16;
  y = (s32)((u32)D_801F51B1[cell[0xB] * 2] << 16);
  *(u32*)(cell + 0x38) = y;
  distance = func_8015477C(*(s32*)(cell + 0x34), y);

  target = D_1F800044;
  ((WorkRecord801F4150*)target)->unk_3E = (u16)(-distance / 2 - D_801F51B4[target[0xB]]);
  target[0xB]++;

  probe = D_1F800044;
  if (func_8015B5D4((u32)&D_80144ED0, D_801F5198[probe[0xB]]) != 0) {
    state = D_1F800044;
    state[2]++;
    setWorkByte9To9AndAdvanceWorkByte2WhenIndexedFlagClearThenDrawFlagIndexedScaledFrameWorld03Area11813_801F4278();
  } else {
    state = D_1F800044;
    state[2] = 3;
    setWorkByte9To9AndAdvanceWorkByte2WhenIndexedFlagSetThenDrawWorld03Area11813_801F43FC();
  }
}

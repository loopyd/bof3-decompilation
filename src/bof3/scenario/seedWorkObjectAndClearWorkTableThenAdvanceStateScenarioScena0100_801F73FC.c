#include "bof3/bof3.h"

extern u8* scenarioScena0100Work; /* @source 0x1F800044 @kind unknown */
/* @source 0x8015477C @kind unknown
 * Shared fixed-point distance helper over the word pair passed as (x, z);
 * callers publish its result scaled by 0x10000.
 */
extern s16 func_8015477C(s32 arg0, s32 arg1);
/* @source 0x8015DF18 @kind unknown
 * Resident front-end sound-cue dispatcher; consumes one u32 cue id.
 */
extern void func_8015DF18(u32 cue_id);
void clearWorkTableAt800E4800ScenarioScena0100_801F7AB8(void);

/* @source 0x801F73FC
 * @behavior Entry 0 of the work-byte-1 handler table at 0x801FE26C, so no
 * in-image jal targets it. Reads the work pointer from scratchpad slot
 * 0x1F800044, stores the position pair 0x600000/0x110000 in its words at
 * offsets 0x34 and 0x38, and publishes the func_8015477C distance of that pair,
 * scaled by 0x10000, in its word at 0x3C. Stores 0x180 and 0x500 in its
 * halfwords at 0x2E and 0x30, clears countdown byte 9, and runs the overlay
 * helper that clears the lead byte of every entry of the 0x800E4800 work
 * table. It then reloads the work pointer, increments work byte 1 modulo 256
 * and queues sound cue 0x202. Each pointer-cell read keeps its own local at the
 * position the original reads it, and the 0x18-byte frame only keeps $ra across
 * its two calls. Field roles stay offset-anchored because their wider meanings
 * are unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkObjectAndClearWorkTableThenAdvanceStateScenarioScena0100_801F73FC(void) {
  u8* work;
  u8* record;
  u8* state;
  s16 distance;

  work = scenarioScena0100Work;
  *(u32*)(work + 0x34) = 0x600000;
  *(u32*)(work + 0x38) = 0x110000;
  distance = func_8015477C(*(s32*)(work + 0x34), 0x110000);
  record = scenarioScena0100Work;
  *(u32*)(record + 0x3C) = distance << 16;
  *(u16*)(record + 0x2E) = 0x180;
  *(u16*)(record + 0x30) = 0x500;
  record[9] = 0;
  clearWorkTableAt800E4800ScenarioScena0100_801F7AB8();
  state = scenarioScena0100Work;
  state[1]++;
  func_8015DF18(0x202);
}

#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. The record's state byte is re-read
 * through this cell because the store into the record may alias it. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Base of the shared main-RAM flag bank probed by the flag tester. */
extern u8 D_80144EA0; /* @source 0x80144EA0 @kind unknown */

s32 func_8015B5D4(u32 arg0, s32 arg1);

/* Local polygon helper that emits the four texture-mapped quads of the work
 * record's position band. */
extern void func_801F2EFC(void); /* @source 0x801F2EFC */

/* Local polygon helper that emits the work record's quad pair at an offset of
 * sixteen from the position word. */
extern void func_801F3064(void); /* @source 0x801F3064 */

/* @source 0x801F3290
 * @behavior Overlay per-frame work-record step: probes flag 5 of the shared
 * main-RAM flag bank at 0x80144EA0 through the shared flag tester
 * 0x8015B5D4. When that flag reads set, the work record published at the
 * scratchpad pointer cell 0x1F800044 has its tick byte at 0x09 cleared
 * (`sb $zero,0x9`) and its state byte at 0x02 bumped by one (`lbu`,
 * `addiu v0,v0,0x1`, `sb`), which is the byte the overlay's state-byte-2
 * dispatcher uses as its handler index; the cell is re-read before the state
 * byte because the intervening store through the record may alias it. Both
 * paths then call the two local polygon helpers at 0x801F2EFC and 0x801F3064.
 * Takes no arguments and returns nothing; the frame of 0x18 bytes holds the
 * return address only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTickAndStepStateWhenFlag5SetThenDrawPolygonsWorld01Area04113_801F3290(void) {
  u8* work;
  u8* step;
  u8 state;

  if (func_8015B5D4((u32)&D_80144EA0, 5) != 0) {
    work = D_1F800044;
    work[9] = 0;
    step = D_1F800044;
    state = step[2];
    step[2] = state + 1;
  }
  func_801F2EFC();
  func_801F3064();
}

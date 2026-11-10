#include "bof3/bof3.h"

typedef struct Area06513WorkRecord {
  u8  unk_00;
  u8  unk_01[0x0A];
  u8  unk_0b;
  u32 unk_0c;
  u32 unk_10;
  u8  unk_14[0x20];
  u32 unk_34;
  u32 unk_38;
} Area06513WorkRecord;

/* @source 0x1F800044 @kind unknown */
extern Area06513WorkRecord* area06513WorkCursor_1F800044;

void func_80196070(void);
void func_8014D978(void);
void func_8014D290(void);

/* @source 0x801F4560
 * @behavior Overlay state handler 2 of the local state-1 handler table
 * 0x801F5320. It reads the work record published at the scratchpad pointer cell
 * 0x1F800044 and, while work byte 0x0B is non-zero, calls the shared helper
 * 0x80196070 and returns as soon as bit 0x80 of work byte 0x00 is set; while
 * work byte 0x0B is zero it stores 1 there when that bit is clear. It then
 * reads the cell again and adds the work word at 0x0C to the word at 0x34 and
 * the work word at 0x10 to the word at 0x38, and finally calls the shared
 * helpers 0x8014D978 and 0x8014D290 in that order. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkPositionPairAndRunSharedUpdatesWorld01Area06513_801F4560(void) {
  Area06513WorkRecord* work;
  Area06513WorkRecord* state;

  work = area06513WorkCursor_1F800044;
  if (work->unk_0b != 0) {
    if ((work->unk_00 & 0x80) != 0) {
      func_80196070();
      return;
    }
  } else if ((work->unk_00 & 0x80) == 0) {
    work->unk_0b = 1;
  }

  state = area06513WorkCursor_1F800044;
  state->unk_34 += state->unk_0c;
  state->unk_38 += state->unk_10;
  func_8014D978();
  func_8014D290();
}

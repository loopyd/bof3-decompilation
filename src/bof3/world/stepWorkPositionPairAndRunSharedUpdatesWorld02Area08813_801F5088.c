#include "bof3/bof3.h"

typedef struct Area08813WorkRecord {
  u8  unk_00;
  u8  unk_01[0x0A];
  u8  unk_0b;
  u32 unk_0c;
  u32 unk_10;
  u8  unk_14[0x20];
  u32 unk_34;
  u32 unk_38;
} Area08813WorkRecord;

void func_80196070(void);
void func_8014D978(void);
void func_8014D290(void);

/* @source 0x801F5088
 * @behavior Overlay state handler 2 of the local state-1 handler table
 *           0x801F624C, selected by the dispatcher 0x801F4CD8 through work
 *           byte 0x01. It reads the work record published at the scratchpad
 *           pointer cell 0x1F800044. While work byte 0x0B is non-zero it calls
 *           the shared helper 0x80196070 and returns as soon as bit 0x80 of
 *           work byte 0x00 is set; while work byte 0x0B is zero it stores 1
 *           there when that bit is clear. It then reads the cell again and
 *           adds the work word at 0x0C to the word at 0x34 and the work word at
 *           0x10 to the word at 0x38, integrating the two deltas into the
 *           position pair, and finally calls the shared helpers 0x8014D978 and
 *           0x8014D290 in that order. It takes no arguments, saves only the
 *           return address at 0x10 of its 0x18-byte frame and returns nothing.
 *           The 0xA4 payload bytes differ from the lifted
 *           stepWorkPositionPairAndRunSharedUpdatesWorld02Area08713_801F4558
 *           in emi/world02/area087/13 at 2 of 164 bytes, both inside the
 *           internal `j` word at span offset 0x3C: its target 0x801F511C sits
 *           +0xB30 from that sibling's 0x801F45EC, exactly this span's address
 *           delta, so the two spans are one body and this source takes that
 *           sibling's statement order with this target's own symbols.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkPositionPairAndRunSharedUpdatesWorld02Area08813_801F5088(void) {
  Area08813WorkRecord* work;
  Area08813WorkRecord* state;

  work = SPAD_PTR_SLOT(Area08813WorkRecord, 0x44);
  if (work->unk_0b != 0) {
    if ((work->unk_00 & 0x80) != 0) {
      func_80196070();
      return;
    }
  } else if ((work->unk_00 & 0x80) == 0) {
    work->unk_0b = 1;
  }

  state = SPAD_PTR_SLOT(Area08813WorkRecord, 0x44);
  state->unk_34 += state->unk_0c;
  state->unk_38 += state->unk_10;
  func_8014D978();
  func_8014D290();
}

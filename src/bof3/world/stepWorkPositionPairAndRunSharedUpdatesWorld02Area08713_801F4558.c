#include "bof3/bof3.h"

typedef struct Area08713WorkRecord {
  u8  unk_00;
  u8  unk_01[0x0A];
  u8  unk_0b;
  u32 unk_0c;
  u32 unk_10;
  u8  unk_14[0x20];
  u32 unk_34;
  u32 unk_38;
} Area08713WorkRecord;

void func_80196070(void);
void func_8014D978(void);
void func_8014D290(void);

/* @source 0x801F4558
 * @behavior Overlay state handler 2 of the local state-1 handler table
 *           0x801F57E0, selected by the dispatcher 0x801F41A8 through work
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
 *           The 0xA4 payload bytes matched the lifted
 *           stepWorkPositionPairAndRunSharedUpdatesWorld01Area06513_801F4560
 *           in emi/world01/area065/13, the lifted
 *           stepWorkPositionPairAndRunSharedUpdatesWorld04Area15213_801F43E8
 *           in emi/world04/area152/13 and the lifted func_801F4528 in
 *           emi/world00/area016/13 at 163 of 164 bytes; the single differing
 *           byte is 0x3C, the low byte of the `j` word, whose target is each
 *           span's own epilogue and therefore shifts by exactly each span's
 *           address delta (+8 for 0x801F4560, -0x170 for 0x801F43E8 and -0x30
 *           for 0x801F4528), so the four spans are one body and this source
 *           takes that family's statement order with this target's own
 *           symbols.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkPositionPairAndRunSharedUpdatesWorld02Area08713_801F4558(void) {
  Area08713WorkRecord* work;
  Area08713WorkRecord* state;

  work = SPAD_PTR_SLOT(Area08713WorkRecord, 0x44);
  if (work->unk_0b != 0) {
    if ((work->unk_00 & 0x80) != 0) {
      func_80196070();
      return;
    }
  } else if ((work->unk_00 & 0x80) == 0) {
    work->unk_0b = 1;
  }

  state = SPAD_PTR_SLOT(Area08713WorkRecord, 0x44);
  state->unk_34 += state->unk_0c;
  state->unk_38 += state->unk_10;
  func_8014D978();
  func_8014D290();
}

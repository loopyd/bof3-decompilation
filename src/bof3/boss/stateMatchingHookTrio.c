#include "bof3/bof3.h"

typedef void (*Boss03416Handler)(void);

/* Party-record byte 0x13C of the 0x140-byte run this tree anchors at
 * 0x80145E90, read here as a standalone cell. */
extern u8 D_80145FCC[]; /* @source 0x80145FCC @kind data */
/* Shared byte the caller sets; sibling sources read it as a field-record index
 * of the 0xA4-byte records at 0x80144968, the same kind of index a work
 * record's byte 0x13C holds. */
extern u8 D_80181B14; /* @source 0x80181B14 @kind data */
/* This overlay's selected party slot, in the reviewed data row D_800C26B8. */
extern u8 D_800C2778; /* @source 0x800C2778 @kind data */

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss03416Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss03416Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss03416Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* Overlay entry points this installer publishes, all still unmatched rows of
 * this payload. */
void func_800C20A8(void); /* @source 0x800C20A8 */
void func_800C22D0(void); /* @source 0x800C22D0 */
void func_800C1F30(void); /* @source 0x800C1F30 */

/* @source 0x800C1EA8
 * @behavior Overlay installer that selects a party slot and then publishes this
 * overlay's battle step handler trio. It loads the shared byte D_80181B14 once,
 * walks the three 0x140-byte party records of the run 0x80145E90 reading each
 * record's byte 0x13C through the cell D_80145FCC, and for every slot whose
 * 0x13C byte equals that shared byte it stores the slot number into this
 * overlay's byte D_800C2778, which the published handlers load as an index. The
 * walk is bounded only by masking the counter to a byte and comparing it
 * against 3, so the record count is never compared against a loaded value. It
 * then stores 0x800C20A8, 0x800C22D0 and 0x800C1F30 into the shared hook slots
 * 0x801463A4, 0x801463A8 and 0x801463AC: the same three slots, in the same
 * order and with the same store shape the exact sibling
 * publishBattleHooksBossBoss03416_800C2340 wrote, so that sibling supplied the
 * tail's statement order while the published addresses come from this span's
 * own bytes. The counter-and-offset loop is the shape of the exact sibling
 * scanWorkRecordsForPartyByte2ScenarioScena0700_801FDA34, which walks the same
 * record run with the same 0x140 scale and reads a different record byte, so it
 * supplied this scan's statement order and this function writes its own cells
 * and bound. The comparison byte and the records sit in the shared main
 * executable and the store target in this payload's data row, so the cells are
 * bound by address. No word of this payload holds this function's address and
 * no jal reaches it, so the main program reaches it through the overlay entry
 * point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateMatchingHookTrio(void) {
  u8 i;
  u32 offset;
  u8 index;

  index = D_80181B14;
  i = 0;
  do {
    offset = i * 0x140;
    if (D_80145FCC[offset] == index) {
      D_800C2778 = i;
    }
    i++;
  } while (i < 3);
  D_801463A4 = func_800C20A8;
  D_801463A8 = func_800C22D0;
  D_801463AC = func_800C1F30;
}

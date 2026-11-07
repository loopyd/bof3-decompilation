#include "bof3/bof3.h"

extern u16 D_80145AA8; /* @source 0x80145AA8 @kind unknown */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind unknown */
extern s32 D_80145EC4; /* @source 0x80145EC4 @kind unknown */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind unknown */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */
extern u16 D_801F4E14; /* @source 0x801F4E14 @kind unknown */

/* Shared main-executable cue dispatcher: walks the cue id in its only
 * argument. Its body is not part of this target. */
void func_8015DF18(u16 arg0);

/* @source 0x801F30B0
 * @behavior Overlay pan step, entry 11 of the local state table based at
 *           0x801F4004 and entry 15 of the table based at 0x801F3FF4, so it
 *           runs while the state byte at +1 of the work record published at
 *           the scratchpad pointer cell 0x1F800044 reads 11 or 15.
 *           It returns untouched unless the shared pad halfword D_80145AA8
 *           carries exactly one face button (0x1000 Triangle, 0x2000 Circle,
 *           0x4000 Cross, 0x8000 Square) or one diagonal pair of two of them
 *           (0x3000, 0x6000, 0x9000, 0xC000). Otherwise it caches the pad in
 *           the overlay's own halfword D_801F4E14 - the cell the continuation
 *           func_801F3304 re-reads for its eight pan frames - plays the
 *           main-executable cue 0x202 through func_8015DF18 and then steps the
 *           party coordinate pair by 0x800 in the pressed direction:
 *           0x1000/0x3000 subtract 0x800 from D_80145EC8 (y), 0x4000/0xC000
 *           add 0x800 to it, 0x2000/0x6000 add 0x800 to D_80145EC4 (x) and
 *           0x8000/0x9000 subtract 0x800 from it. Each arm first republishes
 *           the first 0x140-byte work record D_80145E90 as both the shared
 *           record pointer D_80146250 and the scratchpad work cursor, and the
 *           previously published cursor is restored afterwards, so the step
 *           is written through the party record while the caller's own record
 *           is still the current one. Finally the record byte +0x01, the state
 *           index the local dispatchers select with, is armed with 12 and the
 *           record countdown byte +0x09 with 12, so
 *           countdownScratchStateToMode13World01Area06813_801F32CC runs the
 *           count down before the continuation takes over.
 *           Cell evidence: D_80145EC4 and D_80145EC8 sit 0x34 and 0x38 bytes
 *           into D_80145E90, the coord_x_34/coord_y_38 fields of the shared
 *           GameWorkArea record, and the same pair is the party coordinate
 *           pair of the main executable. The 0x18-byte frame only keeps $ra
 *           and $s0 across the cue call.
 *           Shape evidence, no clone: the immediately following unlifted
 *           neighbour func_801F3304 (0x801F3304) holds the same eight-case
 *           dispatch, the same four arms, the same D_80145E90 publish and the
 *           same saved-cursor restore, with the sign of every 0x800 step
 *           inverted - it is the mirror continuation - so its statement order,
 *           operand forms and the 0x11 scratchpad cell spelling are taken from
 *           it while this function's own directions are written from these
 *           bytes. No byte-identical twin exists: the full 0x21C-byte span
 *           matches no other payload, and its 12-byte leading signature occurs
 *           in none of them.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepCoordinatePairByFaceButtonAndArmState12World01Area06813_801F30B0(void) {
  u8** slots;
  u16* pad;
  u8* previous;

  if (D_80145AA8 == 0x1000 || D_80145AA8 == 0x4000 || D_80145AA8 == 0x2000 ||
      D_80145AA8 == 0x8000 || D_80145AA8 == 0x3000 || D_80145AA8 == 0x6000 ||
      D_80145AA8 == 0x9000 || D_80145AA8 == 0xC000) {
    pad = &D_80145AA8;
    D_801F4E14 = *pad;
    func_8015DF18(0x202);
    switch (*pad) {
    case 0x1000:
    case 0x3000:
      slots = SPAD_PTR_TABLE(u8);
      previous = slots[0x11];
      D_80146250 = D_80145E90;
      slots[0x11] = D_80145E90;
      D_80145EC8 -= 0x800;
      slots = SPAD_PTR_TABLE(u8);
      slots[0x11] = previous;
      break;
    case 0x2000:
    case 0x6000:
      slots = SPAD_PTR_TABLE(u8);
      previous = slots[0x11];
      D_80146250 = D_80145E90;
      slots[0x11] = D_80145E90;
      D_80145EC4 += 0x800;
      slots = SPAD_PTR_TABLE(u8);
      slots[0x11] = previous;
      break;
    case 0x4000:
    case 0xC000:
      slots = SPAD_PTR_TABLE(u8);
      previous = slots[0x11];
      D_80146250 = D_80145E90;
      slots[0x11] = D_80145E90;
      D_80145EC8 += 0x800;
      slots = SPAD_PTR_TABLE(u8);
      slots[0x11] = previous;
      break;
    case 0x8000:
    case 0x9000:
      slots = SPAD_PTR_TABLE(u8);
      previous = slots[0x11];
      D_80146250 = D_80145E90;
      slots[0x11] = D_80145E90;
      D_80145EC4 -= 0x800;
      slots = SPAD_PTR_TABLE(u8);
      slots[0x11] = previous;
      break;
    }
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x01] = 0xC;
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x09] = 0xC;
  }
}

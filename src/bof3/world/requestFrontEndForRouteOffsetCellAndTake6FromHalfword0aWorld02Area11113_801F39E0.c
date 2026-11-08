#include "bof3/bof3.h"

/* 152-byte (0x98) work-record cell of the shared table based at 0x80147A58.
 * Only the stride is proven here: the original scales both the published work
 * record's cell index and the selected cell slot by exactly 0x98, the same
 * stride the overlay's other steps use for these cells. */
typedef struct World02Area11113WorkRecord {
  u8 bytes[0x98];
} World02Area11113WorkRecord;

/* 0x140-byte front-end record rows of the shared main-executable data
 * segment. Only the stride is proven here: the original divides the raw byte
 * distance between the published record pointer and the table base by exactly
 * 0x140 to build the one-based ordinal it passes to the request helper. */
typedef struct World02Area11113FrontEndRecord {
  u8 bytes[0x140];
} World02Area11113FrontEndRecord;

/* Cell coordinates stored in the 0x98-byte work-record cells: 0x80147A8E is
 * byte 0x36 of the cell and 0x80147A92 byte 0x3A, both read with a signed
 * halfword load and a cell index scaled by 0x98. The two are addressed as
 * their own absolute symbols because the original forms each address from one
 * `lui`/`addu` pair against the symbol itself. */
extern u8 D_80147A8E[]; /* @source 0x80147A8E @kind data */
extern u8 D_80147A92[]; /* @source 0x80147A92 @kind data */

/* Selector byte at byte 8 of the same 0x98-byte work-record cells; the
 * original stores the published route byte at 0x80147A60 + slot * 0x98. */
extern u8 D_80147A60[]; /* @source 0x80147A60 @kind table */

/* Shared route-step table: entry i holds the first-coordinate step at
 * D_80181B00 and the second-coordinate step at D_80181B01, two bytes per
 * entry, so the original indexes it with the route byte times two. Both loads
 * are unsigned here. */
extern u8 D_80181B00[]; /* @source 0x80181B00 @kind table */
extern u8 D_80181B01[]; /* @source 0x80181B01 @kind table */

/* Packed cell-code map inside this overlay's trailing data blob: two nibbles
 * per byte, four bytes per map column, so the original addresses it as
 * column * 4 + row / 2. */
extern const u8 D_801F5C18[]; /* @source 0x801F5C18 @kind data */

/* Base of the shared 0x140-byte front-end record table and the pointer cell
 * that publishes the record the front end currently works on. */
extern World02Area11113FrontEndRecord D_80145E90[]; /* @source 0x80145E90 @kind table */
extern World02Area11113FrontEndRecord* D_80146250; /* @source 0x80146250 @kind unknown */

/* Record published through the global pointer cell at 0x80148210; this body
 * reads its halfword at +0x0A, writes it back after the request call and then
 * takes six units off it. */
extern u16* D_80148210; /* @source 0x80148210 @kind unknown */

/* Shared front-end request helper: takes the published record, a work-mode
 * byte, the selected cell slot, a one-byte argument word and a sub-command. */
void func_801AC768(u16* record, s32 mode, u8 slot, u8 argument, s32 command);

/* @source 0x801F39E0
 * @behavior Area step that resolves a packed-map cell from the published work
 *           record instead of the shared scroll pair: it takes the record's
 *           index byte at +0x18, reads the signed coordinate pair stored at
 *           bytes 0x3A and 0x36 of that 0x98-byte work-record cell, halves each
 *           coordinate minus its grid origin (0xB1 for the first, 0xB for the
 *           second) and adds the shared route-step entry selected by the
 *           record route byte at +8 masked with 0x7. While both coordinates
 *           stay below 7 it reads the nibble at
 *           0x801F5C18[col * 4 + ((row & 0xFF) >> 1)], low nibble on an odd row
 *           and high nibble on an even one, and takes that nibble minus one as
 *           the cell slot. While the slot stays below 4 it requests work mode 7
 *           through the shared helper at 0x801AC768 with the cell slot, the
 *           one-based ordinal of the record published at 0x80146250 inside the
 *           0x140-byte table at 0x80145E90 and sub-command 0xC, saving the
 *           published record halfword at +0x0A around the call, publishes the
 *           route byte to byte 8 of the selected 0x98-byte cell at 0x80147A60
 *           and then takes six units off that halfword. Takes no arguments and
 *           returns nothing; the 0x28-byte frame keeps $ra, $s0 and $s1.
 *
 * Name evidence: 0x801F5C18 is the packed cell map and the 0x98-byte cells at
 *           0x80147A58 the work records the overlay's other steps in this
 *           payload already name, the 0x80181B00/0x80181B01 pair is the shared
 *           route-step table the world overlays index by the route byte, and
 *           0x80145E90/0x80146250 are the front-end record table and its
 *           published pointer the neighbouring request step at 0x801F35F4
 *           already names. The route byte and the record halfword at +0x0A keep
 *           their address-anchored roles: this step reads and re-stores the
 *           halfword around the request and the neighbouring step at
 *           0x801F3790 takes two units off the same cell.
 *
 * Shape: no byte-identical twin exists. The same body was screened against
 *           every payload under out/binaries (the full 0x1E0-byte range and the
 *           leading 64 bytes) with no hit, and the near neighbour in this same
 *           payload at 0x801F35F4 is a same-target sibling, which is shape
 *           evidence and never clone eligibility. The body is therefore seeded
 *           from the original bytes: the map lookup half follows 0x801F4510,
 *           and the request tail follows 0x801F35F4 with mode 7, sub-command
 *           0xC and a different post-call store.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestFrontEndForRouteOffsetCellAndTake6FromHalfword0aWorld02Area11113_801F39E0(
    void) {
  s32 colHalf;
  s32 col;
  s32 row;
  u8 route;
  u8 code;
  u8 slot;
  u16 saved;
  u8 ordinal;

  route = SPAD_PTR_SLOT(u8, 0x44)[8] & 0x7;
  colHalf =
      (*(s16*)(D_80147A92 + SPAD_PTR_SLOT(u8, 0x44)[0x18] * 0x98) - 0xB1) / 2;
  col = D_80181B01[route * 2] + colHalf;
  row = (*(s16*)(D_80147A8E + SPAD_PTR_SLOT(u8, 0x44)[0x18] * 0x98) - 0xB) / 2 +
      D_80181B00[route * 2];
  if ((u8)row < 7) {
    col = (u8)col;
    if ((u8)col < 7) {
      if (row & 1) {
        const u8* cell = D_801F5C18 + col * 4;
        code = cell[(u8)row >> 1] & 0xF;
      } else {
        const u8* cell = D_801F5C18 + col * 4;
        code = cell[(u8)row >> 1] >> 4;
      }
      slot = code - 1;
      if (slot < 4) {
        ordinal = (u8)(D_80146250 - D_80145E90 + 1);
        saved = D_80148210[5];
        func_801AC768(D_80148210, 7, slot, ordinal, 0xC);
        D_80148210[5] = saved;
        D_80147A60[slot * 0x98] = SPAD_PTR_SLOT(u8, 0x44)[8] & 0x7;
        D_80148210[5] -= 6;
      }
    }
  }
}

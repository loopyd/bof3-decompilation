#include "bof3/bof3.h"

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

/* Record published through the global pointer cell at 0x80148210: this body
 * stores its step byte at +7 and sets bit 0x40 in its flags byte at +0. */
extern u8* D_80148210; /* @source 0x80148210 @kind unknown */

/* Shared facing halfword, cleared together with the work record's facing word
 * at +0x14, the same pair the reset step at 0x801F564C clears. */
extern u16 D_80149330; /* @source 0x80149330 @kind data */

/* Shared helper taking the published record's route byte. The owning main
 * executable target still carries this range as raw assembly, so the address
 * keeps its address-anchored name. */
void func_801C5DFC(u8 route);

/* @source 0x801F3BC0
 * @behavior Walks the shared route-step table out of the grid cell of the
 *           published work record and publishes the walked step count. It
 *           takes the signed coordinate pair stored at bytes 0x36 and 0x3A of
 *           the record published at scratchpad pointer cell 0x44, subtracts
 *           the grid origins 0xB and 0xB1, halves both as unsigned values, and
 *           masks the record route byte at +8 with 0x7 to select the step pair
 *           at 0x80181B00 and 0x80181B01. It then repeatedly adds those steps
 *           and stops as soon as either truncated coordinate reaches 7 or the
 *           packed nibble of 0x801F5C18 at column times four plus signed row
 *           over two (low nibble on an odd row, high nibble on an even one)
 *           reads 0xF, adding four to a byte counter per pass. When that
 *           counter is not zero it stores the counter plus one into byte 7 of
 *           the record published at 0x80148210, calls the shared helper at
 *           0x801C5DFC with the record route byte, sets bit 0x40 in the
 *           published record's byte 0, clears the shared facing halfword at
 *           0x80149330 and the work record's word at +0x14, and stores -0x500
 *           into the work record's halfword at +0x3E. Takes no arguments and
 *           returns nothing; the 0x18-byte frame keeps $ra.
 *
 * Name evidence: 0x80181B00/0x80181B01 are the shared route-step table the
 *           neighbouring cell step at 0x801F39E0 already names, 0x801F5C18 is
 *           the packed cell map that step and 0x801F4510 read, 0x80148210 is
 *           the published-record pointer cell the overlay's step code writes,
 *           and 0x80149330 is the shared facing halfword the reset step at
 *           0x801F564C clears alongside the same work-record word at +0x14.
 *           The counter is proven only as a search length, so the name says
 *           step count rather than claiming a unit the bytes do not show.
 *
 * Shape: no byte-identical twin exists. The payload never names this address
 *           in a call immediate or a table word, and the near neighbours at
 *           0x801F39E0 and 0x801F4510 are same-target siblings, which is shape
 *           evidence and never clone eligibility. The lookup half follows
 *           0x801F39E0's head, but this body keeps running that lookup in a
 *           loop and halves both coordinates with the unsigned shift rather
 *           than the signed `/ 2` its neighbours use, so the body is seeded
 *           from the original bytes.
 * @status partial
 * @match 36.84
 * @residual 35 matching instructions of 95 original; 376 of 380 bytes.
 *           First difference at 0x801F3BC4: the build saves $s0 to keep the
 *           scratchpad cell address across the helper call, while the original
 *           rematerializes it. The explicit back edge avoids the duplicated
 *           row update, but the map base remains inside the lookup arms and
 *           register allocation and byte-test forms still differ.
 */
void walkRouteUntilBlockedCellAndPublishStepCountWorld02Area11113_801F3BC0(
    void) {
  u8* record;
  const u8* cell;
  u32 row;
  u32 col;
  s32 half;
  u8 route;
  u8 code;
  u8 steps;

  steps = 0;
  record = SPAD_PTR_TABLE(u8)[0x11];
  row = (u32)(*(s16*)(record + 0x36) - 0xB) >> 1;
  col = (u32)(*(s16*)(record + 0x3A) - 0xB1) >> 1;
  route = record[8] & 7;
route_step:
  row += D_80181B00[route * 2];
  if ((u8)row >= 7) {
    goto route_done;
  }
  col += D_80181B01[route * 2];
  if ((u8)col >= 7) {
    goto route_done;
  }
  half = (s8)row / 2;
  if (row & 1) {
    cell = D_801F5C18 + (s8)col * 4;
    code = cell[half] & 0xF;
  } else {
    cell = D_801F5C18 + (s8)col * 4;
    code = cell[half] >> 4;
  }
  if (code == 0xF) {
    goto route_done;
  }
  steps += 4;
  goto route_step;
route_done:
  if (steps) {
    D_80148210[7] = steps + 1;
    func_801C5DFC(SPAD_PTR_TABLE(u8)[0x11][8]);
    D_80148210[0] |= 0x40;
    D_80149330 = 0;
    record = SPAD_PTR_TABLE(u8)[0x11];
    *(u32*)(record + 0x14) = 0;
    *(u16*)(record + 0x3E) = -0x500;
  }
}

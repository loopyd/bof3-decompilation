#include "bof3/bof3.h"

/* One-shot gate byte of this overlay's entry sequence: the prologue clears
 * flag 0x28 of the shared flag bank through 0x8015B5A8 while this byte reads
 * clear and then runs the body only once it reads 1. The overlay step at
 * 0x801F46A8 opens with the same two-read gate. */
extern u8 D_80143F03; /* @source 0x80143F03 @kind data */

/* Shared main-RAM flag bank whose bit 0x28 the prologue sets and the tail
 * probes before it publishes the state bytes. */
extern u8 D_80144F28; /* @source 0x80144F28 @kind data */

/* Work record published at the scratchpad pointer cell: this body reads its
 * route byte at offset 8 to select the shared route-step entry. */
extern u8* g_world02Area11113Work; /* @source 0x1F800044 @kind data */

/* Shared route-step table: entry i holds the first-coordinate step at
 * D_80181B00 and the second-coordinate step at D_80181B01, two bytes per
 * entry, so the original indexes it with the route byte times two. Both reads
 * in this body are signed byte loads, hence the explicit signed cast the
 * neighbouring steps at 0x801F35F4 and 0x801F3BC0 also use. */
extern u8 D_80181B00[]; /* @source 0x80181B00 @kind table */
extern u8 D_80181B01[]; /* @source 0x80181B01 @kind table */

/* The two shared mode bytes the overlay's entry states seed: 0x801448EB is
 * the state selector, 0x801448EC its companion code. */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */
extern u8 D_801448EC; /* @source 0x801448EC @kind data */

/* Shared flag helpers of the resident executable: the bit setter, the bit
 * tester and the front-end startup helper the overlay's entry states call. */
void func_8015B5A8(void* arg0, u8 bit_index);
s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015C088(void);

/* Record-slot selector and state-11 entry of this payload; this body is its
 * only in-payload caller and passes the codes 3, 7, 5 and 1. */
void selectWorkRecordSlotFromScrollCellAndEnterState11World02Area11113_801F4510(
    u8 value);

/* @source 0x801F4140
 * @behavior One-shot position trigger that enters state 11 for the work
 *           record published at scratchpad pointer cell 0x44. While the shared
 *           gate byte D_80143F03 reads clear it sets flag 0x28 of the main-RAM
 *           bank D_80144F28 through 0x8015B5A8, and it returns 0 unless that
 *           byte then reads 1.
 *
 *           It takes the doubled cell coordinate of each argument from `>> 15`
 *           and the cell index from `>> 16`. For each side of two nested
 *           rectangles it tests a cell on the rectangle edge together with the
 *           sign of that record's signed route step, read at 0x80181B00 for
 *           the first coordinate and at 0x80181B01 for the second, indexed by
 *           the record route byte at +8 masked with 0x7. On a match it selects
 *           the record slot and enters state 11 with code 3 (first coordinate
 *           at 0x16 stepping up), 7 (first coordinate at 0x30 stepping down),
 *           5 (second coordinate at 0x162 stepping up) or 1 (second coordinate
 *           at 0x17C stepping down), and with 7, 3, 1 or 5 for the inner
 *           rectangle edges at 0x20, 0x26, 0x16C and 0x172. The 0x26 edge
 *           tests the first coordinate against the second coordinate's
 *           0x16E..0x170 range. When the cell index pair falls inside 0x11..0x12 and
 *           0xB7..0xB8 and flag 0x28 of D_80144F28 reads clear, it runs the
 *           front-end startup helper 0x8015C088 and publishes state selector
 *           0x11 with code 0x14. Every path returns 0; the 0x20-byte frame
 *           keeps $ra, $s0 and $s1.
 *
 * Name evidence: the code values are the argument of this overlay's state-11
 *           entry 0x801F4510, whose own lifted source records that this
 *           payload has no other caller and that 0x801F4140 passes 3, 7, 5 and
 *           1, and that entry publishes the state selector 0x11 to
 *           0x801448EB; the tail here stores that same selector 0x11 and the
 *           code 0x14 itself. The route-step reads are the pair
 *           0x80181B00/0x80181B01 that the neighbouring steps at 0x801F35F4,
 *           0x801F39E0 and 0x801F3BC0 already name as the shared route-step
 *           table, and the record is the work record published at scratchpad
 *           cell 0x44 that this target's map names g_world02Area11113Work. The
 *           two arguments carry no separately evidenced role and no payload
 *           word or `jal` names this function, so they stay structural and the
 *           name keeps only the evidenced cell and route-step test.
 *
 * Shape: no byte-identical twin exists. The full 0x3D0-byte span and its
 *           leading 64 bytes were both scanned across every payload in
 *           out/binaries with no hit, so the clone lever does not apply, and
 *           the discriminating 0x162/0x17C cell immediates appear only in this
 *           payload. The near neighbour at 0x801F46A8 of this same payload is
 *           the shape evidence: it opens with the identical D_80143F03
 *           two-read gate, the same 0x8015B5A8(&D_80144F28, 0x28) call and the
 *           same `bne 1` guard, which fixes the prologue spelling, while its
 *           body is a different walk. The body is therefore seeded from the
 *           original bytes: the eight edge tests repeat one pattern (truncated
 *           coordinate equality, masked range test on the other coordinate,
 *           signed route-step equality against the truncated coordinate minus
 *           an immediate) and the entry call is the 0x801F4510 signature with
 *           a constant code.
 * @status partial
 * @match 68.00
 * @residual 170 matching instructions, 1000 of 976 bytes; first difference
 *           at 0x801F4180 is the gate branch to an epilogue 24 bytes later.
 *           The doubled coordinates occupy the original $a2/$a1 pair and
 *           each signed-halfword subtraction survives, but it executes before
 *           the guard rather than filling the table load's delay slot.
 *           Truncation, constants and table loads use different registers,
 *           leaving load-delay nops and different branch scheduling.
 */
s32 enterState11WithCodeForMatchedWorkCellAndRouteStepWorld02Area11113_801F4140(
    s32 arg0, s32 arg1) {
  s32 x;
  s32 y;
  s32 want;

  if (D_80143F03 == 0) {
    func_8015B5A8(&D_80144F28, 0x28);
  }
  if (D_80143F03 != 1) {
    return 0;
  }

  x = arg0 >> 15;
  y = arg1 >> 15;

  want = (s16)x - 0x15;
  if ((s16)x == 0x16) {
    if ((u16)(y - 0x162) < 0x1B) {
      if ((s8)D_80181B00[(g_world02Area11113Work[8] & 7) * 2] == want) {
        selectWorkRecordSlotFromScrollCellAndEnterState11World02Area11113_801F4510(
            3);
        return 0;
      }
    }
  }
  want = (s16)x - 0x31;
  if ((s16)x == 0x30) {
    if ((u16)(y - 0x162) < 0x1B) {
      if ((s8)D_80181B00[(g_world02Area11113Work[8] & 7) * 2] == want) {
        selectWorkRecordSlotFromScrollCellAndEnterState11World02Area11113_801F4510(
            7);
        return 0;
      }
    }
  }
  want = (s16)y - 0x161;
  if ((s16)y == 0x162) {
    if ((u16)(x - 0x16) < 0x1B) {
      if ((s8)D_80181B01[(g_world02Area11113Work[8] & 7) * 2] == want) {
        selectWorkRecordSlotFromScrollCellAndEnterState11World02Area11113_801F4510(
            5);
        return 0;
      }
    }
  }
  want = (s16)y - 0x17D;
  if ((s16)y == 0x17C) {
    if ((u16)(x - 0x16) < 0x1B) {
      if ((s8)D_80181B01[(g_world02Area11113Work[8] & 7) * 2] == want) {
        selectWorkRecordSlotFromScrollCellAndEnterState11World02Area11113_801F4510(
            1);
        return 0;
      }
    }
  }
  want = (s16)x - 0x21;
  if ((s16)x == 0x20) {
    if ((u16)(y - 0x16E) < 3) {
      if ((s8)D_80181B00[(g_world02Area11113Work[8] & 7) * 2] == want) {
        selectWorkRecordSlotFromScrollCellAndEnterState11World02Area11113_801F4510(
            7);
        return 0;
      }
    }
  }
  want = (s16)x - 0x25;
  if ((s16)x == 0x26) {
    if ((u16)(y - 0x16E) < 3) {
      if ((s8)D_80181B00[(g_world02Area11113Work[8] & 7) * 2] == want) {
        selectWorkRecordSlotFromScrollCellAndEnterState11World02Area11113_801F4510(
            3);
        return 0;
      }
    }
  }
  want = (s16)y - 0x16D;
  if ((s16)y == 0x16C) {
    if ((u16)(x - 0x22) < 3) {
      if ((s8)D_80181B01[(g_world02Area11113Work[8] & 7) * 2] == want) {
        selectWorkRecordSlotFromScrollCellAndEnterState11World02Area11113_801F4510(
            1);
        return 0;
      }
    }
  }
  want = (s16)y - 0x171;
  if ((s16)y == 0x172) {
    if ((u16)(x - 0x22) < 3) {
      if ((s8)D_80181B01[(g_world02Area11113Work[8] & 7) * 2] == want) {
        selectWorkRecordSlotFromScrollCellAndEnterState11World02Area11113_801F4510(
            5);
        return 0;
      }
    }
  }

  if ((u32)(arg0 >> 16) - 0x11 < 2) {
    if ((u32)(arg1 >> 16) - 0xB7 < 2) {
      if (func_8015B5D4((u32)&D_80144F28, 0x28) == 0) {
        func_8015C088();
        D_801448EB = 0x11;
        D_801448EC = 0x14;
      }
    }
  }
  return 0;
}

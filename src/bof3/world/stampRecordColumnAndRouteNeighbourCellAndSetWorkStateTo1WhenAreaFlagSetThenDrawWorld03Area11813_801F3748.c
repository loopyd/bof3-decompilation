#include "bof3/bof3.h"

/* Shared flag tester of the main executable, the same probe the neighbouring
 * handlers of this overlay call. */
s32 func_8015B5D4(u32 arg0, s32 arg1);

/* The overlay record draw routine this step finishes with, called with mode 1.
 * It is a later code span of this same overlay. */
void func_801F381C(s32 arg0);

/* Flag bank of the main executable, probed at the record cursor byte 0x0B. */
extern u8 D_80144ED0; /* @source 0x80144ED0 @kind unknown */

/* Work record pointer cell of the scratchpad. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

/* Resident map base pointer of the main executable, the same cell the overlay
 * steps at 0x801F3FD0 and 0x801F4610 stamp through. It lies outside this
 * overlay's payload and is a D_<ADDRESS> name, so the generated undefined-syms
 * map binds it and no target-map row is needed. */
extern u32 D_8014931C; /* @source 0x8014931C @kind unknown */

/* @source 0x801F3748
 * @behavior Byte-2 state handler that the first byte-2 dispatch table of this
 *           overlay (0x801F516C, indexed by the dispatcher at 0x801F3014)
 *           lists at entry 5. Reads the work record published at the scratchpad
 *           pointer cell 0x1F800044, takes its cursor byte 0x0B as the flag
 *           number and probes that flag in the bank at 0x80144ED0 through the
 *           shared flag tester func_8015B5D4; when the flag is clear only the
 *           draw call runs. When the flag is set it stamps 0xA1 into the map
 *           byte of the record column at offset 0x36 for the row at offset
 *           0x3A, addressed through the resident map base pointer 0x8014931C
 *           scaled by the resident map row stride byte 0x80104000, then stamps
 *           the same 0xA1 into one neighbouring cell chosen by the record
 *           route byte at offset 0x08: the row below when that route byte is
 *           zero, otherwise the cell at the column advanced by the route byte.
 *           It then stores 1 into the record state byte at offset 0x02 so the
 *           next dispatch takes entry 1 of the same table. Every path finishes
 *           with the overlay record draw routine 0x801F381C called with mode 1.
 *           Takes no arguments and returns nothing; the record pointer is
 *           loaded before the frame is created, the 0x18-byte frame only keeps
 *           $ra at 0x10 across the draw call, and the record cell is reloaded
 *           for each of the three uses. The row stride byte is addressed
 *           through one pointer local so a single base serves both stamps, as
 *           the original materialises 0x80104000 once and reloads only the
 *           byte.
 * @status partial
 * @match 65.45
 * @residual The body is 220 bytes against the original 212. The first map
 *           stamp matches, but the second stride load follows the route
 *           branch rather than filling the record reload delay slot. The
 *           second stamp writes the same 0xA1 bits through a signed-byte
 *           lvalue, emitting -95 rather than the original 161. Record, row
 *           and final-address registers also differ in the second stamp.
 */
void stampRecordColumnAndRouteNeighbourCellAndSetWorkStateTo1WhenAreaFlagSetThenDrawWorld03Area11813_801F3748(
    void) {
  u8* record;
  u8* stride;
  s32 col;
  s32 row;

  if (func_8015B5D4((u32)&D_80144ED0, D_1F800044[0xB]) != 0) {
    stride = PSX_PTR(u8, 0x80104000u);
    record = D_1F800044;
    *(u8*)(D_8014931C + FIELD_REF(s16, record, 0x3A) * stride[0] +
           FIELD_REF(s16, record, 0x36)) = 0xA1;

    record = D_1F800044;
    col = FIELD_REF(s16, record, 0x36) + FIELD_REF(u8, record, 0x08);
    row = FIELD_REF(s16, record, 0x3A);
    if (FIELD_REF(u8, record, 0x08) == 0) {
      row = row + 1;
    }
    *(s8*)(D_8014931C + row * stride[0] + col) = -95;

    record = D_1F800044;
    FIELD_REF(u8, record, 0x02) = 1;
  }

  func_801F381C(1);
}

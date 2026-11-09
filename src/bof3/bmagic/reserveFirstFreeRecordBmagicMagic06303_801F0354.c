#include "bof3/bof3.h"

/* Record flags pool at 0x801F0454: 0x60 records of stride 0x20, so the pool
 * ends exactly at 0x801F1054, the work-record pointer cell this overlay's frame
 * loop publishes into. The reset at 0x801EECF8 walks the same pool with one
 * `sb $zero` each to record bytes 0, 1 and 2, and the frame loop at 0x801EEC18
 * keeps the pool base in $s2 while it steps. The base sits inside a `textbin`
 * row that no other map row names; the row in this target's symbols.txt makes
 * every reference to it spell this name. */
extern u8 magic06303RecordFlags[]; /* @source 0x801F0454 @kind data */

/* @source 0x801F0354
 * @behavior Scans the 0x60 records of the pool at 0x801F0454, whose stride is
 *           0x20, for the first one whose flags byte has bit 0 clear, sets that
 *           bit while keeping the other bits of the byte, and returns the record
 *           index; once every record is occupied it returns 0xFF. The original
 *           keeps the mask-free counter in one register and its byte-masked copy
 *           in another, scales the masked copy once per iteration and reuses it
 *           for both the load and the store, re-materializes the pool address
 *           for each of those two accesses, and runs in no stack frame at all.
 *           Both callers mask the result to a byte, compare it with 0xFF and
 *           index the same pool with stride 0x20: 0x801EF0AC, which then seeds
 *           the claimed record's byte 1 with 1 and its word at offset 0x1C with
 *           the work-object pointer from the scratchpad cell 0x1F800044, and
 *           0x801EF1F0, which repeats the call until the pool is full. Takes no
 *           arguments and reads or writes no state outside the pool. The body
 *           has the same shape as reserveFirstFreeRecordBmagicMagic10603_801EF914
 *           and differs from it only in the pool it names.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic06303_801F0354(void) {
  u8 index;
  u8 flags;
  u32 offset;

  for (index = 0; index < 0x60; index++) {
    offset = index * 0x20;
    flags = magic06303RecordFlags[offset];
    if (!(flags & 1)) {
      magic06303RecordFlags[offset] = flags | 1;
      return index;
    }
  }
  return 0xFF;
}

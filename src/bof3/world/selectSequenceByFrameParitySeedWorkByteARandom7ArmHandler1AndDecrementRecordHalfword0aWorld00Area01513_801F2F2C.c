#include "bof3/bof3.h"

extern int rand(void);

/* Scratchpad work-object pointer cell. Every access re-materialises the cell,
 * because the original loads it with its own `lui`/`lw` pair before each of the
 * three record accesses. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
/* Shared frame counter; the original reads it once into a saved register and
 * keeps only bit 1. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind data */
/* Two-entry, two-byte-wide selection record run at the end of this payload: the
 * first byte of each record is the sequence id and the second byte rides along
 * into work byte 0x2A. Splat names the two columns as separate byte bases. */
extern u8 D_801F4104[]; /* @source 0x801F4104 @kind data */
extern u8 D_801F4105[]; /* @source 0x801F4105 @kind data */
/* Shared record pointer whose +0x0A halfword is this step's countdown. */
extern u16* D_80148210; /* @source 0x80148210 @kind unknown */

/* Shared work-sequence selector of the resident EXE image. */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801F2F2C
 * @behavior Overlay step handler: the payload's dispatcher func_801F2EE8
 *           indexes the local word table at 0x801F40FC with work byte 0x04,
 *           whose entry 0 is this function and whose entry 1 is func_801F2E60.
 *           This body reads the shared frame counter at 0x80143E6C once, keeps
 *           bit 0x2 of it and shifts that bit down to 0 or 1, then doubles the
 *           result into the byte offset of the two-entry selection run at
 *           0x801F4104 whose records are (sequence 1, second byte 1) and
 *           (sequence 0, second byte 1). It writes rand() & 7 into work byte
 *           0x0A through the scratchpad pointer cell 0x1F800044, passes the
 *           selected record's first byte to the shared work-sequence selector
 *           func_8014D8D4, copies the selected record's second byte (the same
 *           offset in the 0x801F4105 column) into work byte 0x2A after a fresh
 *           load of the pointer cell, writes 1 into work byte 0x04 so the next
 *           frame dispatches to func_801F2E60, and finally reloads the shared
 *           record pointer at 0x80148210 and subtracts 2 from its +0x0A
 *           halfword. Takes no arguments and returns nothing; its only
 *           in-image reference is the handler-table word at 0x801F40FC, so the
 *           overlay step that dispatches the table is what calls it. The 0x18
 *           frame keeps $s0, which holds the doubled index live across both
 *           calls, and $ra. The neighbouring asm row func_801F2DBC carries the
 *           same 164-byte body and differs only in its own two table-base
 *           immediates (0x801F40F8/0x801F40F9): a payload `cmp` reports 4 of the
 *           164 bytes differing, so that sibling is shape evidence and not a
 *           clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectSequenceByFrameParitySeedWorkByteARandom7ArmHandler1AndDecrementRecordHalfword0aWorld00Area01513_801F2F2C(
    void) {
  u32 slot;

  slot = (D_80143E6C & 2u) >> 1;
  D_1F800044[0xA] = rand() & 7;
  func_8014D8D4(D_801F4104[slot * 2]);
  D_1F800044[0x2A] = D_801F4105[slot * 2];
  D_1F800044[4] = 1;
  D_80148210[5] -= 2;
}

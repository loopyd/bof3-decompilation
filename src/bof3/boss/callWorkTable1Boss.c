#include "bof3/bof3.h"

extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/*
 * Code pointer run at 0x800C1B2C, the workspace-state handler table this
 * dispatcher indexes: slot 0 = 0x800C1848, slot 1 = 0x800C18B8, slot 2 =
 * 0x801E35D8, slot 3 = 0x801E3638, slot 4 = 0x801E3980, slot 5 = 0x801E3A00,
 * slot 6 = 0x800C18F4, slot 7 = 0x801E4AE8, slot 8 = 0x801E4D8C, slot 9 =
 * 0x801E4F64, slot 10 = 0x801E52E8, slot 11 = 0x801E5288, slot 12 = 0x801E3BAC,
 * slot 13 = 0x801E3BD0, slot 14 = 0x801E3BAC, slot 15 = 0x801E4490, slot 16 =
 * 0x800C1938, slot 17 = 0x801E4928. The base sits inside the overlay pointer
 * run the reviewed Splat layout places as the single data row at 0x800C1B18,
 * whose words from 0x800C1B74 on are the inert-handler trio that the overlay's
 * indexed dispatcher at 0x800C19B8 selects from, so only the indexed word is
 * named here and the run stays one span.
 */
extern void (*D_800C1B2C[])(void); /* @source 0x800C1B2C @kind data */

/* @source 0x800C1804
 * @behavior Overlay workspace-state dispatch on work-record byte 1: loads the
 * work-record pointer published at the scratchpad pointer cell 0x1F800044,
 * takes that record's unsigned byte at offset 1, scales it by four and calls
 * the matching entry of this overlay's code pointer run based at 0x800C1B2C
 * (0x800C1848, 0x800C18B8, 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00,
 * 0x800C18F4, 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8, 0x801E5288,
 * 0x801E3BAC, 0x801E3BD0, 0x801E3BAC, 0x801E4490, 0x800C1938, 0x801E4928) with
 * no arguments. The original does not bound the index before the scaled table
 * load, and the slots after the index byte, after the table load and in the
 * return delay slot stay nops. It writes no state and returns whatever the
 * called handler leaves in $v0; the 0x18-byte frame only keeps $ra at 0x10
 * across the indirect call. No jal reaches it and no word of the payload holds
 * its address, so the overlay loader enters it from outside the payload. Same
 * shape as the exact sibling dispatchWorkByte1Table21A4BossBoss02816_800C1BEC,
 * whose 0x44-byte span is byte-identical apart from the scaled table load's low
 * immediate, which names 0x800C21A4 there and 0x800C1B2C here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callWorkTable1Boss(void) {
  D_800C1B2C[D_1F800044[1]]();
}

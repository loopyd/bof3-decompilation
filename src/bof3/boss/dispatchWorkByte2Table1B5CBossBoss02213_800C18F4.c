#include "bof3/bof3.h"

extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/*
 * Code pointer run at 0x800C1B5C, the handler table this dispatcher indexes:
 * entry 0 = 0x801E3BAC, entry 1 = 0x801E3BD0, entry 2 = 0x801E3BAC,
 * entry 3 = 0x801E4490, entry 4 = 0x800C1938, entry 5 = 0x801E4928,
 * entry 6 = 0x800C19F0, entry 7 = 0x800C19F8, entry 8 = 0x800C1A00. The base
 * sits inside the overlay pointer run the reviewed Splat layout places as the
 * single data row at 0x800C1B18, whose words from 0x800C1B74 on are the inert
 * handler trio that the overlay's indexed dispatcher at 0x800C19B8 selects
 * from, so only the indexed word is named here and the run stays one span. The
 * nine words this base indexes end at the payload's own end, 0x800C1B80.
 */
extern void (*D_800C1B5C[])(void); /* @source 0x800C1B5C @kind data */

/* @source 0x800C18F4
 * @behavior Overlay workspace-state dispatch on work-record byte 2: loads the
 * work-record pointer published at the scratchpad pointer cell 0x1F800044,
 * takes that record's unsigned byte at offset 2, scales it by four and calls
 * the matching entry of this overlay's code pointer run based at 0x800C1B5C
 * (0x801E3BAC, 0x801E3BD0, 0x801E3BAC, 0x801E4490, 0x800C1938, 0x801E4928,
 * 0x800C19F0, 0x800C19F8, 0x800C1A00) with no arguments. The original does not
 * bound the index before the scaled table load, and the slots after the index
 * byte, after the table load and in the return delay slot stay nops. It writes
 * no state and returns whatever the called handler leaves in $v0; the 0x18-byte
 * frame only keeps $ra at 0x10 across the call. No jal reaches it: its address
 * appears once in this payload, as the word at file offset 0x344, slot 6 of the
 * 0x800C1B2C workspace-state run the dispatcher at 0x800C1804 indexes, so this
 * overlay reaches it only through that table.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2Table1B5CBossBoss02213_800C18F4(void) {
  D_800C1B5C[D_1F800044[2]]();
}

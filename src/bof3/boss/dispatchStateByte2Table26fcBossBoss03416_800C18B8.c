#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the reviewed `textbin` row D_800C26B8 spans
 * 0x800C26B8 through 0x800C277B. Its first five words are a small record
 * (0x00000101, 0x02020101, 0x03030000, 0x00060006, 0xFFFFFFFF) and the words from
 * 0x800C26CC to 0x800C2774 are in-image code pointers, this payload's functions
 * together with the shared main-executable handlers at 0x801E......; the last word
 * of the row, 0x800C2778, is zero. This dispatcher indexes the run from the
 * interior word 0x800C26FC, the eighteenth word of the row, so the run stays one
 * reviewed row and this declaration binds only that word. From there the
 * thirty-one reachable entries are 0x801E3BAC, 0x801E3BD0, 0x801E3BAC,
 * 0x801E4490, 0x800C18FC, 0x801E4928, 0x800C1940, 0x800C199C, 0x800C1A78,
 * 0x800C1B40, 0x800C1C44, 0x800C1CBC, 0x800C1D40, 0x800C1D98, 0x800C1DA0,
 * 0x800C1DA8, 0x800C1DF4, 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980,
 * 0x801E3A00, 0x801E3B68, 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8,
 * 0x801E5288, 0x800C1E90, 0x800C1E98 and 0x800C1EA0.
 */
extern void (*D_800C26FC[])(void); /* @source 0x800C26FC @kind data */

/* @source 0x800C18B8
 * @behavior Overlay state dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler run based at the interior word 0x800C26FC with
 * no arguments. The original does not bound the index before the scaled table
 * load. It reads no other state and writes none; the pointer-cell load precedes
 * frame creation, the 0x18-byte frame only keeps $ra at 0x10 across the indirect
 * call, and the slots after the index byte and after the table load stay nops.
 * The selected handler's $v0 is left untouched by the epilogue, so this shim
 * returns whatever that handler returns. The payload holds no jal naming
 * 0x800C18B8; the one pointer word that does name it is the run entry at
 * 0x800C26E4, so the overlay loader that steps the pointer run is its caller.
 * Same C shape as dispatchStateByte2Table2064BossBoss03616_800C18B8 of
 * emi/boss/boss036/16, which measured 100 percent in its own lift; the two bodies
 * differ in the table word of the scaled load alone.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table26fcBossBoss03416_800C18B8, D_800C26FC);

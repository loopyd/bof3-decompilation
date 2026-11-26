#include "bof3/emi/dispatchers.h"

/* This overlay's first handler run at 0x800C1C24: twelve code pointers inside
 * the reviewed `textbin` row D_800C1C10, holding this payload's own installer
 * func_800C1848 at entry 0 followed by the shared main-executable handlers
 * 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00, 0x801E3B68,
 * 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8 and 0x801E5288; the runs that
 * start at 0x800C1C54 and 0x800C1C60 follow it in the same row. Splat places
 * that whole row, so it is only referenced here, and this dispatcher's base is
 * the run's first word. The generated undefined_syms_auto.txt row already binds
 * 0x800C1C24, so the declaration needs no map row. */
extern void (*D_800C1C24[])(void); /* @source 0x800C1C24 @kind data */

/* @source 0x800C1804
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 before it creates
 * its frame, takes that record's unsigned byte at offset 1, scales it by four
 * and calls the matching entry of this overlay's handler run based at
 * 0x800C1C24 with no arguments. The original does not bound the index before
 * the scaled table load, and the slots after the index byte, after the table
 * load and in the branch delay slot stay nops. It reads no other state and
 * writes none; the 0x18-byte frame only keeps the return address at 0x10 across
 * the indirect call and the epilogue leaves the selected handler's $v0
 * untouched, so this shim returns whatever that handler returns. No word and no
 * `jal` in this payload holds 0x800C1804, so its caller sits outside the
 * payload; it is the first code span of this overlay, directly behind the four
 * header bytes and ahead of the installer func_800C1848, whose address is
 * entry 0 of the run it indexes. Seeded from the exact sibling
 * callStateTable0Boss of this same payload: one
 * payload cmp over the two 0x44-byte spans leaves a single differing byte, the
 * low byte of the scaled table load's offset, 0x24 here against 0x60 there, so
 * only this target's own table literal is written from these bytes. The
 * same-address twins of other boss overlays, such as
 * dispatchStateByte1Table275cBossBoss03316_800C1804, differ from this span in
 * those same two bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1,
                      dispatchWorkByte1Table2,
                      D_800C1C24);

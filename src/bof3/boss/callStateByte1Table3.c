#include "bof3/emi/dispatchers.h"

/* Interior base of the overlay's pointer run: the run the payload keeps from
 * 0x800C1B34 to 0x800C1BBC sits inside the unreviewed Splat asm span at
 * 0x800C1B28, so only the entry base this dispatcher indexes is named here.
 * From 0x800C1B80 the twelve words are code pointers: 0x800C1934, 0x801E52E8,
 * 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00, 0x801E3B68, 0x801E4AE8,
 * 0x801E4D8C, 0x801E4F64, 0x801E52E8 and 0x801E5288. The generated
 * undefined_syms_auto.txt row for D_800C1B80 already binds the word the scaled
 * index is added to, so this declaration needs no map row. */
extern void (*D_800C1B80[])(void); /* @source 0x800C1B80 @kind data */

/* @source 0x800C18F0
 * @behavior Overlay state dispatch on work-record byte 1 against the entry
 * table at 0x800C1B80: loads the work-record pointer published at the
 * scratchpad pointer cell 0x1F800044, takes that record's unsigned byte at
 * offset 1, scales it by four and calls the matching entry of the twelve-word
 * code pointer run based at 0x800C1B80 (0x800C1934, 0x801E52E8, 0x801E35D8,
 * 0x801E3638, 0x801E3980, 0x801E3A00, 0x801E3B68, 0x801E4AE8, 0x801E4D8C,
 * 0x801E4F64, 0x801E52E8, 0x801E5288) with no arguments. Its first entry is
 * 0x800C1934, this overlay's entry point, which publishes this overlay's
 * handler trio and stores 2 into this same work-record byte; the table fourth
 * of 0x800C1B74 names that entry too. The original does not bound the index
 * before the scaled table load, and the slots after the index byte and after
 * the table load stay nops. It writes no state and leaves the handler's $v0
 * untouched; the pointer-cell load precedes frame creation, and the 0x18-byte
 * frame only keeps $ra at 0x10 across the indirect call. No word of this
 * payload holds 0x800C18F0, so the caller that reaches this entry sits outside
 * the payload. Seeded from the exact sibling dispatchStateByte1Table1b44 of
 * this same overlay at 0x800C1804, which indexes a different state table with
 * this same shared body shape: the two 0x44-byte spans differ in the scaled
 * table load's low immediate, 0x1B44 there against 0x1B80 here, so only this
 * target's own table literal comes from these bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1,
                      callStateByte1Table3,
                      D_800C1B80);

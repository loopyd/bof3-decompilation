#include "bof3/emi/dispatchers.h"

/* This overlay's byte-1 handler run based at 0x800C273C: the last fifteen code
 * pointers of the reviewed `textbin` row D_800C26B8 (0x800C26B8 through
 * 0x800C277B). The row's first five words are a small record (0x00000101,
 * 0x02020101, 0x03030000, 0x00060006, 0xFFFFFFFF), the words from 0x800C26CC to
 * 0x800C2774 are in-image code pointers and the last word, 0x800C2778, is zero.
 * From 0x800C273C the entries are 0x800C1DF4, 0x801E52E8, 0x801E35D8,
 * 0x801E3638, 0x801E3980, 0x801E3A00, 0x801E3B68, 0x801E4AE8, 0x801E4D8C,
 * 0x801E4F64, 0x801E52E8, 0x801E5288, 0x800C1E90, 0x800C1E98 and 0x800C1EA0.
 * 0x800C273C is entry 16 of the run the byte-2 dispatcher indexes, so this base
 * covers that run's entries 16 through 30 and both dispatchers share one
 * reviewed row. The row stays one map entry, so this declaration binds only the
 * word the scaled index is added to.
 */
extern void (*D_800C273C[])(void); /* @source 0x800C273C @kind data */

/* @source 0x800C1DB0
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of this overlay's handler run based at the interior word 0x800C273C with
 * no arguments. The original does not bound the index before the scaled table
 * load. It reads no other state and writes none; the pointer-cell load precedes
 * frame creation, the 0x18-byte frame only keeps $ra at 0x10 across the indirect
 * call, and the slots after the index byte, after the table load and in the
 * branch delay slot stay nops. The epilogue leaves the selected handler's $v0
 * untouched, so this shim returns whatever that handler returns. The payload
 * holds no word or jal naming 0x800C1DB0, so its caller is outside this overlay;
 * the first entry of its own run, 0x800C1DF4, writes 2 into that same record
 * byte. Same C shape as dispatchStateByte2Table26fcBossBoss03416_800C18B8 and
 * callStateTable2714Boss of this target, both of
 * which measured 100 percent in their own lifts; the three bodies differ in the
 * record byte and the table word of the scaled load alone.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1,
                      dispatchStateByte1Table273cBossBoss03416_800C1DB0,
                      D_800C273C);

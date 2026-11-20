#include "bof3/emi/dispatchers.h"

/* This overlay's handler run based at 0x800C2714: the final twenty-five code
 * pointers of the reviewed `textbin` row D_800C26B8 (0x800C26B8 through
 * 0x800C277B). The row's first five words are a small record (0x00000101,
 * 0x02020101, 0x03030000, 0x00060006, 0xFFFFFFFF), the words from 0x800C26CC to
 * 0x800C2774 are in-image code pointers and the last word, 0x800C2778, is zero.
 * From 0x800C2714 the entries are 0x800C1940, 0x800C199C, 0x800C1A78,
 * 0x800C1B40, 0x800C1C44, 0x800C1CBC, 0x800C1D40, 0x800C1D98, 0x800C1DA0,
 * 0x800C1DA8, 0x800C1DF4, 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980,
 * 0x801E3A00, 0x801E3B68, 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8,
 * 0x801E5288, 0x800C1E90, 0x800C1E98 and 0x800C1EA0. 0x800C2714 is the sixth
 * word of the run this target's byte-2 dispatcher indexes, so this base covers
 * that run's entries 6 through 30 and the two dispatchers share one reviewed
 * row. The row stays one map entry, so this declaration binds only the word the
 * scaled index is added to.
 */
extern void (*D_800C2714[])(void); /* @source 0x800C2714 @kind data */

/* @source 0x800C18FC
 * @behavior Overlay state dispatch on work-record byte 3: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 3, scales it by four and calls the matching
 * entry of this overlay's handler run based at the interior word 0x800C2714
 * with no arguments. The original does not bound the index before the scaled
 * table load. It reads no other state and writes none; the pointer-cell load
 * precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 * indirect call, and the slots after the index byte, after the table load and in
 * the branch delay slot stay nops. The epilogue leaves the selected handler's
 * $v0 untouched, so this shim returns whatever that handler returns. The
 * payload holds no jal naming 0x800C18FC; the one pointer word that names it is
 * 0x800C270C, entry 4 of the run based at 0x800C26FC, so the byte-2 dispatcher
 * dispatchStateByte2Table26fcBossBoss03416_800C18B8 enters it when work byte 2
 * is 4, and the overlay loader that steps the same row reaches it as well.
 * Same C shape as dispatchStateByte3Table1ebcBossBoss01216_800C18F8 of
 * emi/boss/boss012/16 and dispatchStateByte3Table26a8BossBoss00816_800C18E4 of
 * emi/boss/boss008/16, both of which measured 100 percent in their own lifts;
 * the three bodies differ in the table word of the scaled load alone.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3,
                      dispatchStateByte3Table2714BossBoss03416_800C18FC,
                      D_800C2714);

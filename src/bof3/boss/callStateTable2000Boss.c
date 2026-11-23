#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the words from 0x800C1F9C through 0x800C208F are
 * the payload's reviewed `textbin` row D_800C1F9C, holding in-image code
 * pointers for this payload's functions and the shared main-executable handlers
 * at 0x801E..... The run is only referenced here. This dispatcher indexes it
 * from the word 0x800C2000, that row's +0x64 word, so its base entry is
 * 0x800C1AEC and the eighteen reachable entries are 0x800C1AEC, 0x801E52E8,
 * 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00, 0x800C1B44, 0x801E4AE8,
 * 0x801E4D8C, 0x801E4F64, 0x801E52E8, 0x801E5288, 0x801E3BAC, 0x801E3BD0,
 * 0x801E3BAC, 0x801E4490, 0x800C1B88 and 0x801E4928. The next word,
 * 0x800C2048, begins the table the dispatcher at 0x800C1C08 indexes.
 */
extern void (*D_800C2000[])(void); /* @source 0x800C2000 @kind data */

/* @source 0x800C1AA8
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 0x01 as the index, scales it by four and
 * calls the handler pointer at that index of the local handler table based at
 * 0x800C2000 with no arguments, leaving the selected handler's $v0 untouched
 * through the epilogue. The original does not bound the index before the scaled
 * table load, and the slots after the index byte and after that load stay nops.
 * The pointer-cell load precedes frame creation; the 0x18-byte frame only keeps
 * $ra at 0x10 across the indirect call, and this body writes no other state.
 * The payload contains no word or jal site naming 0x800C1AA8, so its caller is
 * outside this overlay: it takes no arguments and forwards none. Its 68-byte
 * body is the same 17 instructions as the already-exact siblings
 * callStateTable1Boss and
 * callStateTable2054Boss of this target, whose bytes
 * differ in their own table words alone.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, callStateTable2000Boss, D_800C2000);

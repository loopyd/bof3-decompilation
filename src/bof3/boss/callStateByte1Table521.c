#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the payload's reviewed `textbin` row D_800C1CB0
 * spans 0x800C1CB0 to the payload's last word, and its code pointers start at
 * 0x800C1CD8. This dispatcher indexes the run from the interior word
 * 0x800C1D2C, so the run stays one reviewed row and this declaration binds only
 * the word the scaled index is added to. From 0x800C1D2C the entries are this
 * payload's own 0x800C1A28, the main-executable handlers 0x801E52E8,
 * 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00, 0x801E3B68, 0x801E4AE8,
 * 0x801E4D8C, 0x801E4F64, 0x801E52E8 and 0x801E5288, and the words from
 * 0x800C1D5C on are the three inert handlers 0x800C1AB8, 0x800C1AC0 and
 * 0x800C1AC8 of the index dispatcher's own table run.
 */
extern void (*D_800C1D2C[])(void); /* @source 0x800C1D2C @kind data */

/* @source 0x800C19E4
 * @behavior Second-level overlay dispatch on the work record byte at offset 1:
 * loads the work-record pointer published at the scratchpad pointer cell
 * 0x1F800044, takes that record's unsigned byte at offset 1, scales it by four
 * and calls the matching entry of the handler run from its word 31 at
 * 0x800C1D2C with no arguments, so no argument register is loaded and the
 * callee's value in $v0 is what this function returns. Entry 0 of that run is
 * this overlay's 0x800C1A28 step publisher and the entries after it are
 * main-executable handlers. The original does not bound the index before the
 * scaled table load. It reads no other state and writes none; the pointer-cell
 * load precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10
 * across the indirect call, and the slots after the index byte and after the
 * table load stay nops. No in-payload word equals this address and no
 * lui/addiu pair forms it, so neither a `jal` nor a table word names it and the
 * overlay's outer handler table in the main program is what enters it. Same C
 * shape as the exact sibling callStateByte2Table6
 * at 0x800C18B8 of this payload: the unsigned byte load's own displacement
 * (offset 1 rather than 2) and the table word are the only differing words.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, callStateByte1Table521, D_800C1D2C);

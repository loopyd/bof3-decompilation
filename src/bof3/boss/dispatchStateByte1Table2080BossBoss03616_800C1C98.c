#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the payload's reviewed `textbin` row D_800C2020
 * spans 0x800C2020 through 0x800C20BF. Its first five words are a small record
 * (0x04040000, 0x02020303, 0, 0x06010600, 0xFFFFFFFF) and the thirty-five words
 * from 0x800C2034 to 0x800C20BC are in-image code pointers, this payload's
 * functions together with the shared main-executable handlers at 0x801E......
 * The run is only referenced here. This dispatcher indexes it from the interior
 * word 0x800C2080, its twenty-fifth word, so the run stays one reviewed row and
 * this declaration binds only that word. From there the sixteen reachable
 * entries are 0x800C1CDC, 0x801E52E8, 0x801E35D8, 0x801E3638, 0x800C18B8,
 * 0x800C1A54, 0x801E3B68, 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8,
 * 0x801E5288, 0x800C1B5C, 0x800C1B64, 0x800C1B6C and 0x800C1EE4, which is the
 * last word of the row.
 */
extern void (*D_800C2080[])(void); /* @source 0x800C2080 @kind data */

/* @source 0x800C1C98
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x800C2080 with no arguments.
 * The original does not bound the index before the scaled table load. It reads no
 * other state and writes none; the pointer-cell load precedes frame creation, the
 * 0x18-byte frame only keeps $ra at 0x10 across the indirect call, and the slots
 * after the index byte and after the table load stay nops. The selected
 * handler's $v0 is left untouched by the epilogue, so this shim returns whatever
 * that handler returns. The payload holds neither a word nor a jal naming
 * 0x800C1C98, so the overlay loader that steps the pointer run is its caller.
 * Same shape as the exact sibling dispatchStateByte2Table206cBossBoss03616_800C1A54
 * of this target, whose body differs only in the index byte and the table word of
 * its scaled load, and as the exact cross-target sibling
 * dispatchStateByte1Table2d24BossBoss01816_800C1B84, which reads the same byte
 * through its own table.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table2080BossBoss03616_800C1C98, D_800C2080);

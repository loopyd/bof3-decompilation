#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the words from 0x800C1F9C through 0x800C208F are
 * the payload's reviewed `textbin` row D_800C1F9C, which begins with the eight
 * data words 0x800C1F9C through 0x800C1FB8 that this overlay publishes as its
 * work record row and then holds in-image code pointers for this payload's
 * functions and the shared main-executable handlers at 0x801E..... This
 * dispatcher indexes it from the word 0x800C1FBC, so its base entry is
 * 0x800C1848 and the twelve reachable
 * entries are 0x800C1848, 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980,
 * 0x801E3A00, 0x801E3B68, 0x801E4AE8, 0x801E4D8C, 0x800C18A0, 0x801E52E8 and
 * 0x801E5288. The run is only referenced here.
 */
extern void (*D_800C1FBC[])(void); /* @source 0x800C1FBC @kind data */

/* @source 0x800C1804
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 0x01 as the index, scales it by four and
 * calls the matching entry of this overlay's handler run based at 0x800C1FBC
 * with no arguments, leaving the selected handler's $v0 untouched through the
 * epilogue. The original does not bound the index before the scaled table load,
 * and the slots after the index byte and after that load stay nops. The
 * pointer-cell load precedes frame creation; the 0x18-byte frame only keeps $ra
 * at 0x10 across the indirect call, and this body writes no other state. This
 * body is the payload's first code word after its four-byte entry header and no
 * word or jal site in the payload names 0x800C1804, so its caller is outside
 * this overlay: it takes no arguments and forwards none. Same shape as the
 * already-exact sibling dispatchStateByte1Table2054BossBoss00216_800C1C58 of
 * this target, whose bytes differ in that sibling's own table word alone, and
 * as the already-exact dispatchStateByte1Table2d24BossBoss01816_800C1B84 of
 * boss018/16, which reads the same dispatch byte through its own table.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table1FBCBossBoss00216_800C1804, D_800C1FBC);

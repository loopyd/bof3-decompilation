#include "bof3/emi/dispatchers.h"

/* This overlay's third handler run window: the words from 0x800C1F9C through
 * 0x800C208F are the payload's reviewed `textbin` row D_800C1F9C, which holds
 * the in-image code pointers of this payload's functions and the shared
 * main-executable handlers at 0x801E..... This dispatcher indexes it from the
 * interior word 0x800C2030, so its base entry is 0x801E3BAC and its twelve
 * reachable entries are 0x801E3BAC, 0x801E3BD0, 0x801E3BAC, 0x801E4490,
 * 0x800C1B88, 0x801E4928, 0x800C1C40, 0x800C1C48, 0x800C1C50, 0x800C1C9C,
 * 0x801E52E8 and 0x801E35D8. That window overlaps the sibling table of the
 * dispatcher at 0x800C1C58: its last two words, 0x800C2054 and 0x800C2058, are
 * also the first two entries of that table. This body is itself the run word
 * 0x800C2018, entry 9 of the run based at 0x800C1FEC dispatched by 0x800C18A0,
 * which is its only reference in the payload and the reason its parameters stay
 * structural: that caller passes none.
 */
extern void (*D_800C2030[])(void); /* @source 0x800C2030 @kind data */

/* @source 0x800C1B44
 * @behavior Overlay state dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 0x02 as the index, scales it by four and
 * calls the matching entry of the handler run window based at 0x800C2030 with
 * no arguments, leaving the selected handler's $v0 untouched through the
 * epilogue. The original does not bound the index before the scaled table load,
 * and the slots after the index byte and after that load stay nops. The
 * pointer-cell load precedes frame creation; the 0x18-byte frame only keeps $ra
 * at 0x10 across the indirect call, and this body writes no state. The only
 * payload word that names this entry is the run word 0x800C2018, so the
 * dispatcher at 0x800C18A0 is its caller: it takes no arguments and forwards
 * none. Same shape as the already-exact sibling
 * callStateByte2Table2 of this target, whose span
 * differs from this one in the two low bytes of this table word alone.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateTable2030Boss, callStateTable2030Boss, D_800C2030);

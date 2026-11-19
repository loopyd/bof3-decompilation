#include "bof3/emi/dispatchers.h"

/* This overlay's second handler run: the words from 0x800C1F9C through
 * 0x800C208F are the payload's reviewed `textbin` row D_800C1F9C, which holds
 * the in-image code pointers of this payload's functions and the shared
 * main-executable handlers at 0x801E..... This dispatcher indexes it from the
 * word 0x800C1FEC, so its base entry is 0x800C18E4 and the twelve reachable
 * entries are 0x800C18E4, 0x800C191C, 0x800C1A00, 0x800C1A4C, 0x800C1AA0,
 * 0x800C1AEC, 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00 and
 * 0x800C1B44. The word after that run, 0x800C2018, begins the sibling table of
 * the dispatcher at 0x800C1D00. This body is itself entry 9, the word
 * 0x800C1FE0, of the run based at 0x800C1FBC dispatched by 0x800C1804, which is
 * its only reference in the payload and the reason its parameters stay
 * structural: that caller passes none.
 */
extern void (*D_800C1FEC[])(void); /* @source 0x800C1FEC @kind data */

/* @source 0x800C18A0
 * @behavior Overlay state dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 0x02 as the index, scales it by four and
 * calls the matching entry of this overlay's second handler run based at
 * 0x800C1FEC with no arguments, leaving the selected handler's $v0 untouched
 * through the epilogue. The original does not bound the index before the scaled
 * table load, and the slots after the index byte and after that load stay nops.
 * The pointer-cell load precedes frame creation; the 0x18-byte frame only keeps
 * $ra at 0x10 across the indirect call, and this body writes no state. The only
 * payload word that names this entry is the run word 0x800C1FE0, so the
 * dispatcher at 0x800C1804 is its caller: it takes no arguments and forwards
 * none. Same shape as the already-exact sibling
 * dispatchStateByte1Table1FBCBossBoss00216_800C1804 of this target, whose bytes
 * differ in this span's own index byte and table word alone, and which reads the
 * same work record through the run based at 0x800C1FBC.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table1FECBossBoss00216_800C18A0, D_800C1FEC);

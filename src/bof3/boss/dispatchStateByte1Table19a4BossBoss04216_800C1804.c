#include "bof3/emi/dispatchers.h"

/* The handler run this dispatcher indexes, based at 0x800C19A4: word 5 of the
 * pointer run this payload holds at 0x800C1990. Its word 0 is this payload's own
 * step-setup publisher 0x800C1848 and words 1 through 11 are the shared
 * main-executable handlers 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980,
 * 0x801E3A00, 0x801E3B68, 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8 and
 * 0x801E5288. Past it at 0x800C19D4 the run continues with this overlay's inert
 * handlers 0x800C18E4, 0x800C18EC and 0x800C18F4, which the dispatcher at
 * 0x800C18AC indexes, so this declaration binds only the word the scaled index
 * is added to and that row stays one asm row. The generated
 * undefined_syms_auto.txt row for D_800C19A4 already covers the address, so this
 * declaration takes no map row. */
extern void (*D_800C19A4[])(void); /* @source 0x800C19A4 @kind data */

/* @source 0x800C1804
 * @behavior Overlay step dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, before the
 * frame is created, takes that record's unsigned byte at offset 0x01 as the
 * index, scales it by four and calls that entry of the handler run based at
 * 0x800C19A4 with no arguments. That byte is the step the overlay's own setup
 * publisher 0x800C1848 stores there. The index is not bounded before the scaled
 * table load, and the slots after the index byte and after the table load stay
 * nops. It reads no other state and writes none, and the chosen handler's value
 * survives the epilogue, which only reloads $ra from 0x10 of the 0x18-byte
 * frame, so this shim keeps whatever the handler leaves in the return register.
 * No jal in this payload reaches its address and no payload word holds it, so
 * the overlay loader that installs the work record is its caller and the
 * argument list is empty. This is the body the shared EMI dispatcher macro
 * emits; the already exact sibling dispatchStateByte1Table1ddcBossBoss00716_800C190C
 * of BIN/BOSS/BOSS007.EMI#16 supplied its statement order, while the table base
 * and the record byte are this payload's own. The 0x44-byte span is also
 * byte-identical to the copy at offset 4 of BIN/BOSS/BOSS037.EMI#16, proved with
 * a payload cmp over the whole span.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table19a4BossBoss04216_800C1804, D_800C19A4);

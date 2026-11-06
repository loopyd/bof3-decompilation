#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x800C1DDC: twelve code pointers inside the
 * reviewed `textbin` row D_800C1D78, holding this payload's own func_800C1950
 * followed by the shared main-executable handlers 0x801E52E8, 0x801E35D8,
 * 0x801E3638, 0x801E3980, 0x801E3A00, 0x801E3B68, 0x801E4AE8, 0x801E4D8C,
 * 0x801E4F64, 0x801E52E8 and 0x801E5288. The words before it at 0x800C1DD0
 * (0x800C18F4, 0x800C18FC, 0x800C1904) and the run after it at 0x800C1E0C
 * (0x800C19EC, 0x800C19F4, 0x800C1A04) belong to other dispatchers of this
 * payload, so the base named here is the run's first word. Splat places that
 * whole row, so it is only referenced here. */
extern void (*D_800C1DDC[])(void); /* @source 0x800C1DDC @kind data */

/* @source 0x800C190C
 * @behavior Overlay step dispatch on work-record byte 1: loads the work-record
 * pointer published at the non-volatile scratchpad pointer cell 0x1F800044, takes
 * that record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x800C1DDC with no arguments. The
 * original does not bound the index before the scaled table load, and the slots
 * after the index byte and after the table load stay nops. It reads no other state
 * and writes none; the pointer-cell load precedes frame creation, the 0x18-byte
 * frame only keeps $ra at 0x10 across the indirect call, and the selected handler's
 * $v0 is left untouched by the epilogue, so this shim returns whatever that handler
 * returns. No word or `jal` in this payload names 0x800C190C, so its caller is
 * outside this overlay. The 68-byte body is instruction-for-instruction identical
 * to the already-exact siblings dispatchStateByte1Table2744BossBoss00816_800C1EB0
 * and dispatchStateByte1Table0c60BossBoss03816_800C193C, differing only in the
 * %hi/%lo table-address immediates of the `lui $at` and the following `lw`.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table1ddcBossBoss00716_800C190C, D_800C1DDC);

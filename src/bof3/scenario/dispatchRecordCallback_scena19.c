#include "bof3/bof3.h"

typedef void (*Scena19RecordCallback)(u8* record, u32 arg1);

extern Scena19RecordCallback D_801F6DE0[];
extern u32 D_8014686C;

/* @source 0x801F6D10
 * @behavior Dispatches the per-record callback indexed by the record's unsigned
 * byte field at offset 0x7A through the scena19 overlay's trailing
 * callback-pointer table at 0x801F6DE0, passing the record pointer and the word
 * held in the shared cell at 0x8014686C. It is the scena19 twin of the exact
 * scena18 dispatcher at 0x801F6CAC and of scena00's
 * dispatchRecordCallbackByByte7A; the body saves the return address because the
 * target callback is an ordinary indirect call rather than a sibling tail call,
 * and the shared cell is read before the index is scaled by four.
 *
 * Boundary note: the 64 bytes at 0x801F6D10 are plain code (addiu sp,-0x18;
 * sw ra,0x10(sp); lbu v0,0x7A(a0); lui a1,0x8014; lw a1,0x686C(a1);
 * sll v0,v0,2; lui at,0x801F; addu at,at,v0; lw v0,0x6DE0(at); nop;
 * jalr v0; nop; lw ra,0x10(sp); addiu sp,sp,0x18; jr ra; nop), not a pointer
 * table. The adjacent data it indexes lives at 0x801F6DE0 and 0x801F6DD0:
 * 0x801F6DE0 holds the overlay-local code pointer 0x801F6D50, 0x801F6DE4 holds
 * 0x801F6D68, and the four-entry table at 0x801F6DD0 (0x801F6C40, 0x801F6C54,
 * 0x801F6C68, 0x801F6CA4) is the one func_801F6C40 indexes with `lw 0x6DD0(at)`.
 * Both runs end at the overlay image end 0x801F6DEC and are owned by the
 * neighbouring func_801F6DBC boundary, not by this source.
 * @status unverified
 * @match 0.00
 * @residual none
 */
void dispatchRecordCallback_scena19(u8* record) {
  D_801F6DE0[record[0x7A]](record, D_8014686C);
}

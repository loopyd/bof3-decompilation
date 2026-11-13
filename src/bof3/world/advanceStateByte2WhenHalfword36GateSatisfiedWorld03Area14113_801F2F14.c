#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Spelled as the named fixed address
 * rather than through the constant-address accessor, because the original
 * re-emits `lui $vN,0x1F80` plus the folded `lw $vN,68($vN)` for the store in
 * the shared advance block after the flag tester returns. The `D_<ADDRESS>`
 * spelling binds with no map row.
 * @source 0x1F800044 @kind unknown
 */
extern u8* D_1F800044;

/* Shared state byte the record halfword value 1 of this overlay waits on. The
 * body reads it with `lbu` and compares it against the immediate 1, so it is
 * an unsigned byte: `lbu v1,D_8014933B`, `addiu v0,zero,1`, `beq v1,v0`.
 * @source 0x8014933B @kind data
 */
extern u8 D_8014933B;

/* Main-RAM flag bank words of the two gate arms. Each arm materialises its own
 * word with `lui`/`addiu`, so word 0x80144EF8 (bit 0x14) and word 0x80144F28
 * (bits 0x7A, 0x7B and 0x7C) stay separate bindings even though they sit 0x30
 * bytes apart in the same bank.
 * @source 0x80144EF8 @kind data
 */
extern u8 D_80144EF8;
/* @source 0x80144F28 @kind data */
extern u8 D_80144F28;

/* Shared flag tester of the main executable, outside this payload. The caller
 * passes the flag-bank word and the bit index and a non-zero answer reports the
 * flag as set; this target's generated undefined_funcs_auto.txt binds the
 * address, so the raw `func_` name stays and no map row is owed. */
s32 func_8015B5D4(u32 arg0, s32 arg1);

/* @source 0x801F2F14
 * @behavior State-1 handler of the work-record state-byte-2 dispatch run of this
 *           overlay: the payload's handler table at 0x801F47E8 holds this
 *           address at word 1, so the body has no `jal` caller. It reads the
 *           work record through the scratchpad pointer cell 0x1F800044 and
 *           switches on the record signed halfword at +0x36 (`lh`, guarded by
 *           an unsigned `sltiu` bound of 5, so a negative value returns
 *           untouched as well). Value 0 waits on bit 0x14 of the main-RAM flag
 *           bank word 0x80144EF8 through the shared flag tester 0x8015B5D4,
 *           value 1 waits on the shared state byte 0x8014933B reading 1, and
 *           values 2, 3 and 4 wait on bits 0x7A, 0x7B and 0x7C of the flag bank
 *           word 0x80144F28. When the selected gate answers non-zero the body
 *           re-reads the scratchpad cell and increments the record dispatch
 *           byte at +2 by one; that byte is the index of the state-byte-2
 *           dispatcher at 0x801F2CE8, so the record leaves this handler for the
 *           next arm of the same run. Every other halfword value returns
 *           untouched. Takes no arguments and returns nothing; its 0x18-byte
 *           frame only holds ra at +0x10 and the record pointer load precedes
 *           the frame creation.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceStateByte2WhenHalfword36GateSatisfiedWorld03Area14113_801F2F14(void) {
  switch (*(s16*)(D_1F800044 + 0x36)) {
    case 0:
      if (func_8015B5D4((u32)&D_80144EF8, 0x14) != 0) {
        D_1F800044[2]++;
      }
      break;
    case 1:
      if (D_8014933B == 1) {
        D_1F800044[2]++;
      }
      break;
    case 2:
      if (func_8015B5D4((u32)&D_80144F28, 0x7A) != 0) {
        D_1F800044[2]++;
      }
      break;
    case 3:
      if (func_8015B5D4((u32)&D_80144F28, 0x7B) != 0) {
        D_1F800044[2]++;
      }
      break;
    case 4:
      if (func_8015B5D4((u32)&D_80144F28, 0x7C) != 0) {
        D_1F800044[2]++;
      }
      break;
  }
}

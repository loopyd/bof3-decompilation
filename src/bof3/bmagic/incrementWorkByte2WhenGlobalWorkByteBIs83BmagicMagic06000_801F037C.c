#include "bof3/bof3.h"

/* Shared battle work pointer outside this overlay's image. Splat already binds
 * the address through this target's generated undefined_syms_auto.txt, and the
 * bytes read here are byte-addressed, so the local declaration keeps the byte
 * view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F037C
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 and, only when
 *           that work record's byte at offset 0xB equals 0x83, reads the
 *           scratchpad work-object pointer cell at 0x1F800044 once and
 *           increments its byte at offset 0x2 with a byte-width add
 *           (lbu/addiu/sb). The comparison materialises 0x83 into a register
 *           and takes the untaken side of the branch to the epilogue, so the
 *           scratchpad cell is dereferenced only on the taken path. Nothing is
 *           returned, no other state is read or written, and the function has
 *           no frame.
 *
 *           The payload word at 0x801F0BD4 holds this entry address, so the
 *           body is reached as index 6 of the handler run based at 0x801F0BBC
 *           that the work-byte-2 dispatcher at 0x801EFF88 indexes with the
 *           work object byte at offset 0x2, not by a direct call. Its 64 bytes
 *           were compared equal to the same overlay's function at 0x801F0090,
 *           which had measured exact, so the statement order, the byte-width
 *           increment and the shared pointer declaration are that sibling's,
 *           with this span's own address and guard immediate (0x83) following
 *           the original bytes here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenGlobalWorkByteBIs83BmagicMagic06000_801F037C(void) {
  if (D_801EB4E0[0xB] == 0x83) {
    SPAD_PTR_SLOT(u8, 0x44u)[2]++;
  }
}

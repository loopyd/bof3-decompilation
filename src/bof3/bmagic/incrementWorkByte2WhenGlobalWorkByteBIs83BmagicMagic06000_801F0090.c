#include "bof3/bof3.h"

/* Shared battle work pointer outside this overlay. Splat already binds the
 * address through this target's generated undefined_syms_auto.txt, and the
 * bytes read here are byte-addressed, so the local declaration keeps the byte
 * view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F0090
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 and, only when
 * that work record's byte at offset 0xB equals 0x83, reads the scratchpad
 * work-object pointer cell at 0x1F800044 once and increments its byte at offset
 * 0x2 with a byte-width add (lbu/addiu/sb). Nothing is returned, no other state
 * is read or written, and the scratchpad cell is dereferenced only on the taken
 * path. All 64 bytes are byte-identical to func_801F037C in this same overlay,
 * and the body differs from the exact sibling
 * incrementWorkByte2WhenGlobalWorkByteBIs84BmagicMagic11303_801EF8A4
 * (emi/bmagic/magic113/03@0x801EF8A4) in the guard immediate alone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenGlobalWorkByteBIs83BmagicMagic06000_801F0090(void) {
  if (D_801EB4E0[0xB] == 0x83) {
    SPAD_PTR_SLOT(u8, 0x44u)[2]++;
  }
}

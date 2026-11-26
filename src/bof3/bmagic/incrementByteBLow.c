#include "bof3/bof3.h"

/* Shared battle work pointer outside this overlay, already resolved by its
 * generated undefined-symbol map. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF148
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 once and
 *           compares that record's byte at offset 0xB against two with an
 *           unsigned bound (lbu/sltiu), so the guard is taken for every value
 *           below two. Only on that path it loads the scratchpad work pointer
 *           cell at 0x1F800044 with its own lui/lw pair and increments the byte
 *           at offset 2 of that work object modulo 256 with lbu/addiu/sb,
 *           sharing the loaded base for the read and the store. Takes no
 *           arguments, returns nothing and keeps no frame, so the scratchpad
 *           cell is dereferenced only when the guard succeeds. Word 3 of the
 *           nine-word handler table based at 0x801EFC2C (0x801EF050, 0x801EF0B8,
 *           0x801EF0F8, 0x801EF148, 0x801EF18C, 0x801EF404, 0x801EF47C,
 *           0x801EF628, 0x801EF6E0) names this address, so the payload holds no
 *           jal caller for it and this target's map leaves the run as the
 *           textbin row workByte1HandlerTable. Bytes 0x801EF148..0x801EF18B
 *           were byte-identical to the then-exact lifts 0x801EF5EC of
 *           bmagic/magic088/03, 0x801EFBA8 of bmagic/magic004/03 and
 *           0x801EF410 of bmagic/magic167/03, and to the then-assembled row
 *           0x801EF078 of bmagic/magic126/03, so statement order, operand forms
 *           and declaration style are copied from those lifts. This target's
 *           map binds no name for the scratchpad cell at 0x1F800044, so the cell
 *           is spelled at its fixed address, the form its magic004 twin measured
 *           exact.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementByteBLow(void) {
  if (D_801EB4E0[0xB] < 2u) {
    SPAD_PTR_SLOT(u8, 0x44u)[2]++;
  }
}

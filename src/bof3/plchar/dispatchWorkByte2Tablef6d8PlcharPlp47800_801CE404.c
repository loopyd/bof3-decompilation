#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: twenty code pointers, 0x801CF6D8 through 0x801CF724,
 * holding this payload's functions plus the shared main-executable handler
 * 0x801B2D54 at entries 2, 5 and 11. The Splat `textbin` row T_801CF6D8 places the
 * words, so the run is only referenced here. This dispatcher indexes it from its
 * first word, so its base is entry 0 and the entries the run still holds from there
 * are 0x801CE448, 0x801CE514, 0x801B2D54, 0x801CE664, 0x801CE758, 0x801B2D54,
 * 0x801CE898, 0x801CEBE4, 0x801CED80, 0x801CF2C0, 0x801CF38C, 0x801B2D54,
 * 0x801CF4CC, 0x801CF5E4, 0x801CE404, 0x801CE620, 0x801CF27C, 0x801CE610,
 * 0x801CE854 and 0x801CF488. */
extern void (*T_801CF6D8[])(void); /* @source 0x801CF6D8 @kind data */

/* Seeded from the lift of the same body at plp478/00 0x801CE854. A payload
 * comparison of this span's 68 bytes at offset 4 against that span found one
 * differing word, the scaled table load's immediate, where each body names its own
 * run; every other word agreed. The statement order, the macro spelling and the
 * declaration style were carried over with this function's own table symbol and
 * its own name.
 */
/* @source 0x801CE404
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801CF6D8 with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load, and that table
 *           load reads the word at the run's base, so entry 0 is 0x801CE448. It
 *           reads no other state and writes none; the pointer-cell load precedes
 *           frame creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte, the slot after the
 *           table load and the slot after the indirect call stay nops. Its only
 *           in-image reference is the word 0x801CE404 at 0x801CF710, entry 14 of
 *           the twenty-word run based at 0x801CF6D8, so the overlay loader that
 *           steps that run is what enters it: it has no jal caller and takes no
 *           arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2Tablef6d8PlcharPlp47800_801CE404, T_801CF6D8);

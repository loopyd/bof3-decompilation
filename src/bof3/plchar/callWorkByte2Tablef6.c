#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: twenty code pointers, the words 0x801CF6D8 through
 * 0x801CF724, holding this payload's functions plus the shared main-executable
 * handler 0x801B2D54 at entries 2, 5 and 11. The Splat `textbin` row workByte2HandlerRun
 * places the words, so the run is only referenced here. This dispatcher indexes it
 * from 0x801CF6E4, so its base is entry 3 of that run and entry 0 is 0x801CE664;
 * the entries the run still holds from there are 0x801CE664, 0x801CE758,
 * 0x801B2D54, 0x801CE898, 0x801CEBE4, 0x801CED80, 0x801CF2C0, 0x801CF38C,
 * 0x801B2D54, 0x801CF4CC, 0x801CF5E4, 0x801CE404, 0x801CE620, 0x801CF27C,
 * 0x801CE610, 0x801CE854 and 0x801CF488. */
extern void (*D_801CF6E4[])(void); /* @source 0x801CF6E4 @kind data */

/* Seeded from the lift of the same body at plp478/00 0x801CE404. A payload
 * comparison of this span's 68 bytes at offset 0x220 against that span found one
 * differing word, the scaled table load's immediate at offset 0x24, where each body
 * names its own run base; every other word agreed. The statement order, the macro
 * spelling and the declaration style were carried over with this function's own
 * table symbol and its own name.
 */
/* @source 0x801CE620
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801CF6E4 with no arguments, discarding any result. The original does
 *           not bound the index before the scaled table load, so that load reads
 *           the word at the run base and entry 0 is 0x801CE664. It reads no other
 *           state and writes none; the pointer-cell load precedes frame creation,
 *           the 0x18-byte frame only keeps $ra at 0x10 across the indirect call,
 *           and the slots after the index byte, after the table load, after the
 *           indirect call and after the return jump all stay nops. Its only
 *           in-image reference is the word 0x801CE620 at 0x801CF714, entry 15 of
 *           the run based at 0x801CF6D8 and entry 12 of this table, so the loader
 *           that steps that run enters it: it has no jal caller and takes no
 *           arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callWorkByte2Tablef6, callWorkByte2Tablef6, D_801CF6E4);

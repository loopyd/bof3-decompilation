#include "bof3/emi/dispatchers.h"

/* This overlay's handler run starts at 0x800C1B58 and holds in-image code
 * pointers for this payload's functions interleaved with the shared
 * main-executable handlers at 0x801E.... The dispatcher indexes it from its
 * first word, so its base entry is 0x800C1848, this overlay's setup routine,
 * then 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00, 0x801E3B68,
 * 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8 and 0x801E5288. The run
 * continues to the end of the payload with the tables the two indexed
 * dispatchers of this overlay select from. It all sits inside the 0x800C1B44
 * `textbin` row, which the reviewed Splat layout places, so it is only
 * referenced here; the generated undefined_syms_auto.txt row for D_800C1B58
 * already binds the word the scaled index is added to.
 */
extern void (*D_800C1B58[])(void); /* @source 0x800C1B58 @kind data */

/* @source 0x800C1804
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 * pointer cell at 0x1F800044, takes that work record's dispatch byte at offset
 * 0x01 as the index and calls the handler pointer at that index of the local
 * handler table based at 0x800C1B58. The selected handler's $v0 is left
 * untouched by the epilogue, so this shim's result is whatever that handler
 * returns. The original does not bound the index before the scaled table load,
 * and the slots after the index byte and after that load stay nops. The
 * pointer-cell load precedes frame creation; the 0x18-byte frame only keeps $ra
 * at 0x10 across the indirect call. The payload names 0x800C1804 in no word and
 * no jal site, so its caller is outside this overlay. Takes no arguments and
 * forwards none. Same shape as the dispatchers at 0x800C1EB0 and 0x800C1804 of
 * BIN/BOSS/BOSS008.EMI#16 and at 0x800C1B84 of BIN/BOSS/BOSS018.EMI#16, whose
 * own metadata recorded the same 17-instruction, 68-byte body; only the table
 * word in the scaled load differs here.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, callStateByte1Table523, D_800C1B58);

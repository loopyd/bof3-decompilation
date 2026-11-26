#include "bof3/emi/dispatchers.h"

/* Overlay-local run of in-image code pointers starting at 0x801D3918: nine words
 * (0x801D3918..0x801D3938) holding 0x801D0C48, 0x801D0CC0, 0x801D0D54,
 * 0x801D0DD8, 0x801D0F10, 0x801D0FEC, 0x801D106C, 0x801D12A0 and 0x801D13F4.
 * The halfword grid entries from 0x801D393C onward belong to the pattern-grid
 * table, not to this run. The Splat `textbin` row at payload offset 0x2D18 owns
 * those words, so the run is only referenced here. This dispatcher indexes it
 * from 0x801D3918, while the ten-word run at 0x801D3954 and the eleven-word run
 * at 0x801D397C of the same blob are indexed by the two later dispatchers. */
extern void (*stepHandlerTable[])(void); /* @source 0x801D3918 @kind data */

/* @source 0x801D0C04
 * @behavior First per-frame step dispatcher of this overlay: it loads the
 * work-record pointer from the scratchpad pointer cell 0x1F800044
 * (`lui $v0,0x1F80` then `lw $v0,0x44($v0)`) before it creates its frame, takes
 * that record's unsigned byte at offset 1 (`lbu $v0,0x1($v0)`), scales it by
 * four (`sll $v0,$v0,2`) and invokes that entry of the overlay-local
 * code-pointer run at 0x801D3918 (`lui $at,%hi` / `addu $at,$at,$v0` /
 * `lw $v0,%lo($at)` / `jalr $v0`) with no arguments and no result consumed. The
 * original does not bound the index before the scaled table load. It reads no
 * state beyond that byte and writes none; its 0x18-byte frame exists only to
 * keep $ra at 0x10 across the indirect call, and the delay slot after the index
 * byte stays a nop. No word in this payload addresses 0x801D0C04, so nothing in
 * the overlay references it directly; it is reached from outside, like the
 * dispatchers of the two later runs, which measured exact from the same body
 * shape (0x801D2170 and 0x801D2DEC).
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, callBeamStepHandler, stepHandlerTable);

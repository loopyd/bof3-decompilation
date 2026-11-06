#include "bof3/emi/dispatchers.h"

/* Second overlay-local run of in-image code pointers, starting at 0x801D397C:
 * eleven words (0x801D397C..0x801D39A4) holding 0x801D21B4, 0x801D23A0,
 * 0x801D2474, 0x801D26BC, 0x801D2934, 0x801D2E30, 0x801D2FD8, 0x801D30AC,
 * 0x801D32F4, 0x801D34F0 and 0x801D369C. The run opens with the same five
 * handlers as the ten-word run at 0x801D3954 and then continues past it instead
 * of following it into the 0x801D2B44 group; the zeros from 0x801D39A8 end the
 * pointer data. The Splat `textbin` row at payload offset 0x2D18 owns those
 * words, so the run is only referenced here. */
extern void (*D_801D397C[])(void); /* @source 0x801D397C @kind data */

/* @source 0x801D2DEC
 * @behavior Per-frame step dispatcher of this overlay, indexing the second
 * handler run: it loads the work-record pointer from the scratchpad pointer cell
 * 0x1F800044 (`lui $v0,0x1F80` then `lw $v0,0x44($v0)`) before it creates its
 * frame, takes that record's unsigned byte at offset 1 (`lbu $v0,0x1($v0)`),
 * scales it by four (`sll $v0,$v0,2`) and invokes that entry of the overlay-local
 * code-pointer run at 0x801D397C (`lui $at,%hi` / `addu $at,$at,$v0` /
 * `lw $v0,%lo($at)` / `jalr $v0`) with no arguments and no result consumed. The
 * original does not bound the index before the scaled table load. It reads no
 * state beyond that byte and writes none; its 0x18-byte frame exists only to keep
 * $ra at 0x10 across the indirect call, and the delay slot after the index byte
 * stays a nop. No word in this payload addresses 0x801D2DEC, so nothing in the
 * overlay references it directly; like the 0x801D2170 entry it is reached from
 * outside the overlay.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerScenarioSce15ef100_801D2DEC, D_801D397C);

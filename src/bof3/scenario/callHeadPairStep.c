#include "bof3/emi/dispatchers.h"

/* Overlay-local run of in-image code pointers starting at 0x801D396C: four words
 * (0x801D396C..0x801D3978) holding 0x801D2BA8, 0x801D2C00, 0x801D2C94 and
 * 0x801D2D5C — the same four handler words that end the ten-word run at
 * 0x801D3954, read here from the tail of it as their own base. The Splat
 * `textbin` row at payload offset 0x2D18 owns those words, so the run is only
 * referenced here. */
extern void (*D_801D396C[])(void); /* @source 0x801D396C @kind data */

/* @source 0x801D2B64
 * @behavior Third per-frame step dispatcher of this overlay, indexing the
 * four-word handler run at 0x801D396C: it loads the work-record pointer from the
 * scratchpad pointer cell 0x1F800044 (`lui $v0,0x1F80` then `lw $v0,0x44($v0)`)
 * before it creates its frame, takes that record's unsigned byte at offset 1
 * (`lbu $v0,0x1($v0)`), scales it by four (`sll $v0,$v0,2`) and invokes that
 * entry of the run (`lui $at,0x801D` / `addu $at,$at,$v0` / `lw $v0,0x396C($at)` /
 * `jalr $v0`) with no arguments and no result consumed. The original does not
 * bound the index before the scaled table load. It reads no state beyond that
 * byte and writes none; its 0x18-byte frame exists only to keep $ra at 0x10
 * across the indirect call, and the delay slot after the index byte stays a nop.
 * No word in this payload holds the address 0x801D2B64, so nothing in the
 * overlay references it directly; like the 0x801D2170 and 0x801D2DEC entries it
 * is reached from outside the overlay.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, callHeadPairStep, D_801D396C);
